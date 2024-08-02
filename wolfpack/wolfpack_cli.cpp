#include <chrono>
#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <tl/expected.hpp>

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
using WolfPackError = std::runtime_error;

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
        if (result == EXIT_SUCCESS) {
            vout << result->output << "\n";
        } else {
            std::cerr << result->output << "\n";
            std::cerr << fmt::format("Shell command '{}' failed with code {}!\n", result->command, result->code);
        }
        return std::move(*result);
    }
    throw WolfPackError(fmt::format("Unexpected IO error: {}\n", result.error()));
};

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
        ("h,help", "Print usage");

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    std::cout << "Running wolfpack...\n";

    const auto project_folder = fs::current_path();
    const auto config_file = project_folder / "wolfpack.json";
    const auto wolfpack_folder = project_folder / ".wolfpack";

    const auto should_pull = result["pull"].as<bool>();

#ifdef NDEBUG
    const auto verbose = result["verbose"].as<bool>();
#else
    const auto verbose = true;
#endif

    if (verbose) {
        vout = OStreamOrNull(&std::cout);
    }

    if (!fs::exists(config_file) || !fs::is_regular_file(config_file)) {
        std::cout << "No wolfpack.json file found. Nothing to do.\n";
        return 0;
    }

    vout << fmt::format("Input folder is: {}\n", project_folder);
    vout << fmt::format("Output folder (wolfpack clone directory) is: {}\n", wolfpack_folder);

    if (wolfpack_folder.has_extension()) {
        throw WolfPackError(fmt::format("Output '{}' needs to be a folder!", wolfpack_folder));
    }

    if (fs::create_directory(wolfpack_folder)) {
        vout << fmt::format("Created output folder at {}\n", wolfpack_folder);
    }

    if (!fs::exists(wolfpack_folder) || !fs::is_directory(wolfpack_folder)) {
        throw WolfPackError(fmt::format("Failed to create output folder '{}'!", wolfpack_folder));
    }

    // wolfpack.json parsing
    std::stringstream config_stream;
    config_stream << std::ifstream(config_file).rdbuf();
    nlohmann::json config_json = nlohmann::json::parse(config_stream);

    struct LibRepo {
        std::string author;
        std::string repo_name;
        std::string tag = "master";

        LibRepo(const std::string& name)
        {
            if (name.empty()) {
                throw WolfPackError("Library name cannot be empty!");
            }
            const size_t slash = name.find('/');
            if (slash == std::string::npos || name.starts_with('/') || name.ends_with('/')) {
                throw WolfPackError(fmt::format("Library name '{}' does not have <author>/<repo_name> format!",
                    name));
            }

            this->author = name.substr(0, slash);
            this->repo_name = name.substr(slash + 1);
        }
    };

    std::vector<LibRepo> libs {};
    libs.reserve(100);

    try {
        if (!config_json.contains("libs")) {
            throw WolfPackError("No libs key found");
        }

        for (auto& [name, options] : config_json["libs"].items()) {
            LibRepo lib(name);
            if (options.contains("tag")) {
                lib.tag = options["tag"];
            }
            libs.emplace_back(lib);

            vout << fmt::format("Will process library {} with tag '{}'...\n", name, lib.tag);
        }
    } catch (std::exception& ex) {
        throw WolfPackError(fmt::format("Parse error of '{}' -> {}", config_stream.str(), ex.what()));
    }

    if (run_command_logged("git --version") == EXIT_FAILURE) {
        throw WolfPackError("Git is not installed! It is needed.");
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

            const auto git_url = fmt::format("https://github.com/{}/{}", lib.author, lib.repo_name);
            if (git_url.find(' ') != std::string::npos) {
                return fmt::format("Git url '{}' cannot contain spaces!", git_url);
            }

            if (run_command_logged(fmt::format("git clone {} {} --depth 1 --recursive", git_url, repo_folder))
                == EXIT_FAILURE) {
                return fmt::format("Failed to clone repo '{}' to '{}'", git_url, repo_folder);
            }

            if (run_command_logged(fmt::format("git -C {} fetch --tags", repo_folder)) == EXIT_FAILURE) {
                return fmt::format("Failed to checkout repo tags of repo {}/{}", lib.author, lib.repo_name);
            }
        } else {
            if (should_pull) {

                if (run_command_logged(fmt::format("git -C {} config pull.rebase true", repo_folder)) == EXIT_FAILURE) {
                    return fmt::format("Failed to set config option for repo {}/{}", lib.author, lib.repo_name);
                }

                if (run_command_logged(fmt::format("git -C {} pull", repo_folder)) == EXIT_FAILURE) {
                    return fmt::format("Failed to pull repo {}/{}", lib.author, lib.repo_name);
                }
            }
        }

        if (run_command_logged(fmt::format("git -C {} checkout {}", repo_folder, lib.tag)) == EXIT_FAILURE) {
            return fmt::format("Failed to checkout branch/tag {} in repo {}/{}", lib.tag, lib.author,
                lib.repo_name);
        }

        // TODO:
        return ""s; // no errors occured :)
    };

    std::vector<std::future<std::string>> tasks {};
    for (const auto& lib : libs) {
        tasks.emplace_back(std::async(std::launch::async, SyncLibRepo, lib));
    }

    bool done;
    bool tasksFailed = false;
    std::vector<bool> readyTasks(tasks.size());
    do {
        for (auto i = 0; i < tasks.size(); i++) {
            done = true;
            if (!readyTasks[i]) {
                done = false;

                if (auto status = tasks[i].wait_for(10ms); status == std::future_status::ready) {
                    if (auto error = tasks[i].get(); !error.empty()) {
                        std::cerr << fmt::format("Task failed with error: {}\n", error);
                        tasksFailed = true;
                    }
                }
                readyTasks[i] = true;
            }
        }
    } while (!done);

    return tasksFailed ? EXIT_FAILURE : EXIT_SUCCESS;
}
}

#if defined(NDEBUG) || 1
#define CATCH_EXCEPTIONS
#endif

auto main(int argc, char** argv) -> int
{
#ifdef CATCH_EXCEPTIONS
    try {
#endif
        return wolfpack::run_with_args(argc, argv);
#ifdef CATCH_EXCEPTIONS
    } catch (std::exception& ex) {
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
#endif
}