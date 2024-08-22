#include <fmt/format.h>
#include <fstream>
#include <stdexcept>

#include "config_reader.hpp"
#include "utils.hpp"

namespace wolfpack {

std::shared_ptr<IConfigReader> ConfigReaders::ReadFile(const std::filesystem::path& path) const
{
    try {
        std::stringstream config_stream;
        config_stream << std::ifstream(path).rdbuf();
        const auto ext = path.extension().string();
        auto& reader = readers.at(ext);
        reader->Parse(config_stream.rdbuf()->str());
        return reader;
    } catch (std::out_of_range& ex) {
        throw WolfPackError(fmt::format("Failed to create reader for file type (format unsupported?): {}: {}", path.extension().string(), ex.what()));
    } catch (std::exception& ex) {
        throw WolfPackError(fmt::format("Error reading config file: {}: {}", path.string(), ex.what()));
    }
}

}