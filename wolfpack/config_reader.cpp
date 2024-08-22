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
        auto reader = readers.at(ext).get();
        reader->Parse(config_stream.rdbuf()->str());
    } catch (std::out_of_range& ex) {
        throw WolfPackError(fmt::format("Failed to create reader for file type: {}", path.extension().string()));
    } catch (std::exception& ex) {
        throw WolfPackError(fmt::format("Error reading config file: {}: {}", path.string(), ex.what()));
    }
}
