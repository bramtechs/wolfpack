#include <iostream>
#include <stdexcept>
#include <ostream>
#include <fmt/format.h>
#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <list>
#include <future>
#include <nlohmann/json.hpp>

#include "utils.hpp"

template <>
class fmt::formatter<std::filesystem::path>
{
public:
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const std::filesystem::path &path, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", path.string());
    }
};

namespace wolfpack
{
    namespace fs = std::filesystem;
    using WolfPackError = std::runtime_error;

    class OStreamOrNull
    {
    public:
        OStreamOrNull(std::ostream *inner = nullptr)
            : inner(inner)
        {
        }

        template <typename T>
        OStreamOrNull &operator<<(const T &data)
        {
            if (inner)
            {
                (*inner) << data;
            }
            return *this;
        }

    private:
        std::ostream *inner;
    };

    static OStreamOrNull vout = {};

    auto get_default_clone_dir() -> fs::path
    {
        const auto home_dir = std::getenv("HOME");
        if (!home_dir)
        {
            throw WolfPackError("HOME environment variable is not set!");
        }
        return fs::path(home_dir) / ".wolfpack";
    }

    auto run_wolfpack(int argc, char **argv) -> void
    {
        cxxopts::Options options("wolfpack", "Wolfpack downloader and synchronizer");

        options.add_options()("i,input", "Your project folder where wolfpack.json is located", cxxopts::value<fs::path>()->default_value(fs::current_path().string())) //
            ("o,output", "Folder where repos are cloned.", cxxopts::value<fs::path>()->default_value(get_default_clone_dir().string()))                                //
            ("no-pull", "Do not pull existing repos")                                                                                                                  //
            ("v,verbose", "Print more info")                                                                                                                           //
            ("h,help", "Print usage");

        const auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help() << std::endl;
            std::exit(0);
        }

        auto input_folder = result["input"].as<fs::path>().lexically_normal();
        const auto output_folder = result["output"].as<fs::path>().lexically_normal();

#ifdef NDEBUG
        const auto verbose = result["verbose"].as<bool>();
#else
        const auto verbose = true;
#endif

        if (verbose)
        {
            vout = OStreamOrNull(&std::cout);
        }

        if (input_folder.filename() == "wolfpack.json")
        {
            input_folder = input_folder.parent_path();
        }

        if (!fs::exists(input_folder) || !fs::is_directory(input_folder))
        {
            throw WolfPackError(fmt::format("Input folder '{}' does not exist!", input_folder));
        }

        const auto input_file = input_folder / "wolfpack.json";
        if (!fs::exists(input_file) || !fs::is_regular_file(input_file))
        {
            throw WolfPackError(fmt::format("Config file '{}' does not exist!", input_file));
        }

        vout << fmt::format("Input folder is: {}\n", input_folder);
        vout << fmt::format("Output folder (wolfpack clone directory) is: {}\n", output_folder);

        if (output_folder.has_extension())
        {
            throw WolfPackError(fmt::format("Output '{}' needs to be a folder!", output_folder));
        }

        if (fs::create_directory(output_folder))
        {
            vout << fmt::format("Created output folder at {}\n", output_folder);
        }

        if (!fs::exists(output_folder) || !fs::is_directory(output_folder))
        {
            throw WolfPackError(fmt::format("Failed to create output folder '{}'!", output_folder));
        }

        // wolfpack.json parsing
        std::stringstream config_stream;
        config_stream << std::ifstream(input_file).rdbuf();
        nlohmann::json config_json = nlohmann::json::parse(config_stream);

        struct LibRepo
        {
            std::string name;
            std::string tag = "master";

            LibRepo(const std::string &name)
                : name(name)
            {
                if (name.empty())
                {
                    throw WolfPackError("Library name cannot be empty!");
                }
                if (name.find('/') == std::string::npos || name.starts_with('/') || name.ends_with('/'))
                {
                    throw WolfPackError(fmt::format("Library name '{}' does not have <author>/<repo_name> format!", name));
                }
            }
        };

        std::list<LibRepo> libs{};

        try
        {
            if (!config_json.contains("libs"))
            {
                throw WolfPackError("No libs key found");
            }

            for (auto &[name, options] : config_json["libs"].items())
            {
                LibRepo lib(name);
                if (options.contains("tag"))
                {
                    lib.tag = options["tag"];
                }
                libs.emplace_back(lib);

                vout << fmt::format("Will process library {} with tag '{}'...\n", name, lib.tag);
            }
        }
        catch (std::exception &ex)
        {
            throw WolfPackError(fmt::format("Parse error of '{}' -> {}", config_stream.str(), ex.what()));
        }

        auto run_command_logged = [verbose](const std::string &command) -> CommandResult
        {
            CommandResult result = run_command(command);
            if (result)
            {
                vout << result.output.str() << "\n";
            }
            else
            {
                std::cerr << result.output.str() << "\n";
                std::cerr << fmt::format("Shell command '{}' failed with code {}!\n", result.command, result.code);
            }
            return result;
        };

        if (!run_command_logged("git --version"))
        {
            throw WolfPackError("Git is not installed!");
        }

        // run async tasks
    }
}

#if defined(NDEBUG) || 1
#define CATCH_EXCEPTIONS
#endif

auto main(int argc, char **argv) -> int
{
#ifdef CATCH_EXCEPTIONS
    try
    {
#endif
        wolfpack::run_wolfpack(argc, argv);
        return 0;
#ifdef CATCH_EXCEPTIONS
    }
    catch (std::exception &ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
#endif
}
