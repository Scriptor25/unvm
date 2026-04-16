#pragma once

#include <optional>
#include <string>
#include <vector>

namespace unvm
{
    struct VersionEntry
    {
        std::string Version;
        std::string Date;
        std::vector<std::string> Files;
        std::optional<std::string> Npm;
        std::string V8;
        std::optional<std::string> Uv;
        std::optional<std::string> ZLib;
        std::optional<std::string> OpenSSL;
        std::optional<std::string> Modules;
        std::optional<std::string> Lts;
        bool Security{};
    };

    using VersionTable = std::vector<VersionEntry>;
}
