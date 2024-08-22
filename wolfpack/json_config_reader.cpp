#include "json_config_reader.hpp"
#include "utils.hpp"

#include <fmt/format.h>

namespace wolfpack {

    using namespace std::string_literals;

std::string JsonConfigReader::GetFileExtension() const
{
    return ".json"s;
};

std::optional<int> JsonConfigReader::GetVersion() const
{
    if (json.contains("version"s)) {
        const auto& value = json.get("version"s);
        if (!value.is<double>()) {
            throw WolfPackError("Config 'version' must be a number."s);
        }
        // Retrieving the value as int causes linking error.
        return static_cast<int>(value.get<double>());
    }
    return std::nullopt;
};

ConfigLibsMap JsonConfigReader::GetLibrariesMap() const
{
    const auto& libs = json.get("libs"s);
    if (!libs.is<picojson::object>()) {
        throw WolfPackError("Config 'libs' must be of type object."s);
    }

    ConfigLibsMap map;
    for (const auto& [key, value] : libs.get<picojson::object>()) {
        auto& entry = map[key];
        if (!value.is<picojson::object>()) {
            throw WolfPackError("Config 'libs' child is not of type object"s);
        }
        for (const auto& [skey, svalue] : value.get<picojson::object>()) {
            entry[skey] = svalue.get<std::string>();
        }
    }

    return map;
}

bool JsonConfigReader::ContainsKey(const std::string& key) const
{
    return json.contains(key);
}

void JsonConfigReader::Parse(const std::string& content)
{
    std::string err = picojson::parse(json, content);
    if (!err.empty()) {
        throw WolfPackError(fmt::format("Json parse error: {}", err));
    }
};

}