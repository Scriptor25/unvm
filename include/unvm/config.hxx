#pragma once

#include <optional>
#include <string>
#include <unordered_set>

namespace unvm
{
    struct Config
    {
        bool Dirty{};

        std::unordered_set<std::string> Installed;
        std::optional<std::string> Default;

        std::optional<std::string> Active;

        std::unordered_set<std::string> Fingerprints;
    };
}
