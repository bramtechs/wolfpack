#include "config_reader.hpp"

using WolfPackError = std::runtime_error;

IConfigReader* ConfigReaders::ReadFile(const std::filesystem::path& path) const
{
    try {
        auto reader = readers.at(path.extension());
        return reader.get();
    } catch (std::out_of_range& ex) {
        throw WolfPackError(fmt::format("Failed to create reader for file type: {}", path.extension()));
    }
}
