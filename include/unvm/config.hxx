#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>

namespace unvm
{
    struct Config
    {
        std::set<std::string> Installed;
        std::map<std::string, unsigned> Active;
        std::optional<std::string> Default;
    };
}
