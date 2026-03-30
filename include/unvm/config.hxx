#pragma once

#include <filesystem>
#include <optional>
#include <set>

namespace unvm
{
    struct Config
    {
        std::filesystem::path InstallDirectory;
        std::filesystem::path ActiveDirectory;

        std::set<std::string> Installed;
        std::optional<std::string> Active;
    };
}
