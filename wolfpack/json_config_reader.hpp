#pragma once

#include "config_reader.hpp"
#include "picojson.h"

namespace wolfpack {

class JsonConfigReader final : public IConfigReader {
public:
    std::string GetFileExtension() const override;
    std::optional<int> GetVersion() const override;
    ConfigLibsMap GetLibrariesMap() const override;
    bool ContainsKey(const std::string& key) const override;
    void Parse(const std::string& content) override;

private:
    picojson::value json {};
};

}