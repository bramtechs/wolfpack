#include <chrono>
#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <future>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <forward_list>
#include <tl/expected.hpp>

#include "config_reader.hpp"
#include "json_config_reader.hpp"
#include "utils.hpp"

// TODO: switch to spdlog

template <>
class fmt::formatter<std::filesystem::path> {
public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const std::filesystem::path& path, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", path.string());
    }
};

namespace wolfpack {
    namespace fs = std::filesystem;

    using namespace std::chrono_literals;
    using namespace std::string_literals;

    class OStreamOrNull {
    public:
        OStreamOrNull(std::ostream* inner = nullptr)
            : inner(inner)
        {
        }

        template <typename T>
        OStreamOrNull& operator<<(const T& data)
        {
            if (inner) {
                (*inner) << data;
            }
            return *this;
        }

    private:
        std::ostream* inner;
    };

    static OStreamOrNull vout = {};

    [[nodiscard]] static auto run_command_logged(const std::string& command) -> CommandResult
    {
        auto future = run_command(command);
        future.wait();
        auto result = future.get();
        if (result.has_value()) {
            if (*result) {
                vout << result->output << "\n";
            }
            else {
                std::cerr << result->output << "\n";
                std::cerr << fmt::format("Shell command '{}' failed with code {}!\n", result->command, result->code);
            }
            return std::move(*result);
        }
        throw WolfPackError(fmt::format("Unexpected IO error: {}\n", result.error()));
    };

    struct LibRepo {
        std::string author;
        std::string repo_name;
        std::string url;
        std::string tag = "master";

        LibRepo(const std::string& name, const std::optional<std::string>& customUrl)
        {
            // parse name
            if (name.empty()) {
                throw WolfPackError("Library name cannot be empty!");
            }
            const size_t slash = name.find('/');
            if (slash == std::string::npos || name.starts_with('/') || name.ends_with('/')) {
                throw WolfPackError(fmt::format("Library name '{}' does not have <author>/<repo_name> format!", name));
            }

            this->author = name.substr(0, slash);
            this->repo_name = name.substr(slash + 1);
            this->url = customUrl.value_or(fmt::format("https://github.com/{}/{}", author, repo_name));
        }
    };

    auto read_libs_from_config(const std::filesystem::path& path) -> std::forward_list<LibRepo> 
    {
        //
        const auto config_readers = ConfigReaders()
                                         .With<JsonConfigReader>();
        const auto config_reader = config_readers.ReadFile(path);

        std::forward_list<LibRepo> libs;

        if (!config_reader->ContainsKey("libs")) {
            throw WolfPackError("No libs key found");
        }

        for (auto& [name, options] : config_reader->GetLibrariesMap()) {
            std::optional<std::string> url {};
            if (options.contains("url")) {
                url = options["url"];
            }
            LibRepo lib(name, url);
            if (options.contains("tag")) {
                lib.tag = options["tag"];
            }
            libs.emplace_front(lib);

            vout << fmt::format("Will process library {} with tag '{}'...\n", name, lib.tag);
        }

        return libs;
    }

    auto get_default_clone_dir() -> fs::path
    {
        const auto home_dir = std::getenv("HOME");
        if (!home_dir) {
            throw WolfPackError("HOME environment variable is not set!");
        }
        return fs::path(home_dir) / ".wolfpack";
    }

    [[nodiscard]] auto run_with_args(int argc, char** argv) -> int
    {
        cxxopts::Options options("wolfpack", "Wolfpack downloader and synchronizer");

        options.add_options() //
            ("pull", "Pull/update existing repos") //
            ("v,verbose", "Print more info") //
            ("check-config", "Check config for errors and do nothing.", cxxopts::value<fs::path>()->default_value("")) //
            ("h,help", "Print usage");

        const auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        const auto& checked_config_path = result["check-config"].as<fs::path>();
        if (!checked_config_path.empty()) {
            auto count = 0;
            for (const auto& lib : read_libs_from_config(checked_config_path)) {
                std::cout << "Library: " << lib.author << " / " << lib.repo_name << "\n"
                          << "-- tag: " << lib.tag << "\n-- url: " << lib.url << '\n';
                count++;
            }
            std::cout << "\nFound " << count << " defined libraries in config file.\n";
            return 0;
        }

        const auto project_folder = fs::current_path();
        const auto wolfpack_folder = project_folder / ".wolfpack";

        const auto should_pull = result["pull"].as<bool>();

#ifdef NDEBUG
        const auto verbose = result["verbose"].as<bool>();
#else
        const auto verbose = true;
#endif

        if (verbose) {
            std::cout << "Running wolfpack...\n";
            vout = OStreamOrNull(&std::cout);
        }

        // locate config file
        fs::path config_file{};
        for (auto& entry : fs::directory_iterator(project_folder)) {
            const auto& entry_path = entry.path();
            if (entry_path.filename() == "wolfpack" && entry_path.has_extension()) {
                config_file = entry_path;
                break;
            }
        }

        if (config_file.empty() || !fs::is_regular_file(config_file)) {
            std::cout << "No wolfpack config file found. Nothing to do.\n";
            return 0;
        }

        vout << fmt::format("Input folder is: {}\n", project_folder);
        vout << fmt::format("Output folder (wolfpack clone directory) is: {}\n", wolfpack_folder);

        if (fs::exists(wolfpack_folder) && !fs::is_directory(wolfpack_folder)) {
            throw WolfPackError(fmt::format("Output '{}' needs to be a folder!", wolfpack_folder));
        }

        if (fs::create_directory(wolfpack_folder)) {
            vout << fmt::format("Created output folder at {}\n", wolfpack_folder);
        }

        if (!fs::exists(wolfpack_folder) || !fs::is_directory(wolfpack_folder)) {
            throw WolfPackError(fmt::format("Failed to create output folder '{}'!", wolfpack_folder));
        }

        const auto libs = read_libs_from_config(config_file);

        if (!run_command_logged("git --version")) {
            throw WolfPackError("Git is not installed! It is required.");
        }

        // run async tasks
        const auto SyncLibRepo = [&wolfpack_folder, should_pull](const LibRepo& lib) -> std::string {
            const auto repo_folder = wolfpack_folder / lib.author / lib.repo_name;
            if (repo_folder.string().find(' ') != std::string::npos) {
                return fmt::format("Folders with spaces are not supported yet, sorry: '{}'", repo_folder);
            }

            if (fs::create_directories(repo_folder)) {
                vout << fmt::format("Created directory {}\n", repo_folder);

                std::cout << fmt::format("Cloning repo {}/{}...\n", lib.author, lib.repo_name);

                if (lib.url.find(' ') != std::string::npos) {
                    return fmt::format("Git url '{}' cannot contain spaces!", lib.url);
                }

                if (!run_command_logged(fmt::format("git clone {} {} --depth 1 --recursive", lib.url, repo_folder))) {
                    return fmt::format("Failed to clone repo '{}' to '{}'", lib.url, repo_folder);
                }

                if (!run_command_logged(fmt::format("git -C {} fetch --tags", repo_folder))) {
                    return fmt::format("Failed to fetch repo tags of repo {}/{}", lib.author, lib.repo_name);
                }
            }
            else {
                if (should_pull) {

                    if (!run_command_logged(fmt::format("git -C {} config pull.rebase false", repo_folder))) {
                        return fmt::format("Failed to set config option for repo {}/{}", lib.author, lib.repo_name);
                    }

                    if (!run_command_logged(fmt::format("git -C {} pull", repo_folder))) {
                        return fmt::format("Failed to pull repo {}/{}", lib.author, lib.repo_name);
                    }
                }
            }

            if (!run_command_logged(fmt::format("git -C {} checkout {}", repo_folder, lib.tag))) {
                std::cerr << fmt::format("Failed to checkout branch/tag {} in repo {}/{}, fetching tags again...\n", lib.tag, lib.author, lib.repo_name);

                if (!run_command_logged(fmt::format("git -C {} fetch --tags", repo_folder))) {
                    return fmt::format("Failed to fetch repo tags of repo {}/{}", lib.author, lib.repo_name);
                }

                if (!run_command_logged(fmt::format("git -C {} checkout {}", repo_folder, lib.tag))) {
                    return fmt::format("Failed to checkout branch/tag {} in repo {}/{}", lib.tag, lib.author, lib.repo_name);
                }
            }

            // TODO:
            return ""s; // no errors occured :)
            };

        std::vector<std::future<std::string>> tasks{};
        for (const auto& lib : libs) {
            tasks.emplace_back(std::async(std::launch::async, SyncLibRepo, lib));
        }

        bool tasksFailed = false;
        for (auto i = 0; i < tasks.size(); i++) {
            tasks[i].wait();
            if (const std::string error = tasks[i].get(); !error.empty()) {
                std::cerr << fmt::format("Task failed with error: {}\n", error);
                tasksFailed = true;
            }
        }

        int code = tasksFailed ? EXIT_FAILURE : EXIT_SUCCESS;
        if (verbose) {
            fmt::println("wolfpack exiting with code {} -> {}", code, code == EXIT_SUCCESS ? "SUCCESS" : "FAILURE");
        }
        return code;
    }
}

#if defined(NDEBUG)
#define CATCH_EXCEPTIONS
#endif

auto main(int argc, char** argv) -> int
{
#ifdef CATCH_EXCEPTIONS
    try {
#endif
        return wolfpack::run_with_args(argc, argv);
#ifdef CATCH_EXCEPTIONS
    }
    catch (std::exception& ex) {
        std::cerr << "wolfpack error: " << ex.what() << std::endl;
        return 1;
    }
#endif
}
