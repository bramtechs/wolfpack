#pragma once
#include <string>
#include <sstream>

namespace wolfpack
{
    struct CommandResult
    {
        int code;
        std::string command;
        std::stringstream output;

        operator bool() const
        {
            return code == EXIT_SUCCESS;
        }
    };

    auto run_command(const std::string) -> CommandResult;

}