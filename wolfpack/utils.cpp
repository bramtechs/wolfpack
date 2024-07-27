#include "utils.hpp"

#include <iostream>
#include <cstdio>
#include <array>
#include <sstream>
#include <fmt/format.h>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace wolfpack {
    static auto run_async_task(const std::string &command) -> tl::expected<CommandResult, std::string> {
        std::string error;
        std::stringstream output_stream;

        CommandResult cmd_result{
            .code = EXIT_FAILURE,
            .command = command + " 2>&1"
        };

        std::cout << fmt::format("--> {}\n", cmd_result.command);

        if (FILE *pipe = popen(cmd_result.command.c_str(), "r")) {
            try {
                std::array<char, 128> buffer{};
                while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                    output_stream << buffer.data();
                }
                cmd_result.code = pclose(pipe);
            } catch (std::exception &ex) {
                pclose(pipe);
                error = fmt::format("Process read error occured. {}\n", ex.what());
            }
        } else {
            error = "Failed to start process!";
        }

        if (!error.empty()) {
            return tl::make_unexpected(error);
        }
        cmd_result.output = output_stream.str();
        return cmd_result;
    };

    auto run_command(const std::string &command) -> std::future<tl::expected<CommandResult, std::string> > {
        return std::async(std::launch::async, run_async_task, command);
    }
}
