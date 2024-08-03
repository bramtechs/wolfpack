#pragma once
#include <string>
#include <future>
#include <tl/expected.hpp>

namespace wolfpack
{
    struct CommandResult
    {
        int code;
        std::string command;
        std::string output;

        operator bool() const {
            return code == EXIT_SUCCESS;
        }
    };

    auto run_command(const std::string &) -> std::future<tl::expected<CommandResult, std::string> >;
}
