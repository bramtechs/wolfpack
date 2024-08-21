#include <fstream>
#include <fmt/format.h>
#include <stdexcept>
#include "config_reader.hpp"

using WolfPackError = std::runtime_error;

IConfigReader* ConfigReaders::ReadFile(const std::filesystem::path& path) const
{
    try {
        std::stringstream config_stream;
        config_stream << std::ifstream(path).rdbuf();
        const auto ext = path.extension().string();
        return readers.at(ext).get();
    } catch (std::out_of_range& ex) {
        throw WolfPackError(fmt::format("Failed to create reader for file type: {}", path.extension().string()));
    } catch (std::exception& ex) {
        throw WolfPackError(fmt::format("Error reading config file: {}: {}", path.string(), ex.what()));
    }
}
