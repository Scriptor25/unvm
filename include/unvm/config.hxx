#pragma once

#include <optional>
#include <set>
#include <string>

namespace unvm
{
    struct Config
    {
        std::set<std::string> Installed;
        std::optional<std::string> Default;

        std::optional<std::string> Active;
    };
}
