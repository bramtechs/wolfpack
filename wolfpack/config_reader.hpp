#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>

namespace wolfpack {

using ConfigLibsMap = std::unordered_map<std::string,
    std::unordered_map<std::string, std::string>>;

class IConfigReader {
public:
    virtual ~IConfigReader() = default;

    virtual std::string GetFileExtension() const = 0;

    virtual std::optional<int> GetVersion() const = 0;

    virtual bool ContainsKey(const std::string& key) const = 0;

    virtual ConfigLibsMap GetLibrariesMap() const = 0;

    virtual void Parse(const std::string& content) = 0;
};

class ConfigReaders {
public:
    template <typename T>
    ConfigReaders& With()
    {
        const auto r = std::make_shared<T>();
        const auto ext = r->GetFileExtension();
        readers.emplace(ext, r);
        return *this;
    }

    std::shared_ptr<IConfigReader> ReadFile(const std::filesystem::path& path) const;

private:
    std::unordered_map<std::string, std::shared_ptr<IConfigReader>> readers {};
};

}