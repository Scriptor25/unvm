#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>

namespace unvm
{
    struct Config
    {
        std::set<std::string> Installed;
        std::map<std::string, std::set<std::string>> Active;
        std::optional<std::string> Default;
    };
}
