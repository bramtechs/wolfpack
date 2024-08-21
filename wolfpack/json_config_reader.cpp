#include "json_config_reader.hpp"

std::string JsonConfigReader::GetFileExtension() const
{
    return "json";
};

std::optional<int> JsonConfigReader::GetVersion() const
{
    throw "todo";
};

ConfigLibsMap JsonConfigReader::GetLibrariesMap() const
{
    throw "todo";
}

bool JsonConfigReader::ContainsKey(const std::string& key) const
{
    throw "todo";
}
;

