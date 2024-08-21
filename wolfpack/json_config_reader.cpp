#include "config_reader.hpp"

class JsonConfigReader : IConfigReader
{
    std::string GetFileExtension() const override {
        return "json";
    };

    std::optional<int> GetVersion() const override {
        throw "todo";
    };

    ConfigsLibsMap GetLibrariesMap() const override {
        throw "todo";
    };
};
