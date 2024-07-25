#include "utils.hpp"
#include <iostream>
#include <cstdio>
#include <array>
#include <fmt/format.h>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace wolfpack
{
    auto run_command(const std::string command) -> CommandResult
    {
        CommandResult cmd_result{};
        cmd_result.code = EXIT_FAILURE;
        cmd_result.command = command + " 2>&1";

        std::cout << fmt::format("--> {}\n", cmd_result.command);

        FILE *pipe = popen(cmd_result.command.c_str(), "r");
        if (pipe)
        {
            try
            {
                std::array<char, 128> buffer;
                while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
                {
                    cmd_result.output << buffer.data();
                }
                cmd_result.code = pclose(pipe);
            }
            catch (std::exception &ex)
            {
                pclose(pipe);
                std::cerr << fmt::format("Process read error occured. {}\n", ex.what());
            }
        }
        else
        {
            std::cerr << "Failed to start process!\n";
        }

        return cmd_result;
    }
}