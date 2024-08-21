#pragma once

#include "config_reader.hpp"

class JsonConfigReader final : public IConfigReader
{
public:
    std::string GetFileExtension() const override;
    std::optional<int> GetVersion() const override;
    ConfigLibsMap GetLibrariesMap() const override;
    bool ContainsKey(const std::string& key) const override;
};
