#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace unvm
{
    struct Config
    {
        std::optional<std::string> Default;
        std::unordered_set<std::string> Installed;
        std::unordered_set<std::string> Fingerprints;
        std::unordered_map<std::string, std::string> Tracked;

        std::optional<std::string> Active;
        std::optional<std::string> Detected;

        bool DefaultDirty;
        std::unordered_set<std::string> AddedVersions;
        std::unordered_set<std::string> RemovedVersions;
        std::unordered_set<std::string> AddedFingerprints;
        std::unordered_set<std::string> RemovedFingerprints;
        std::unordered_set<std::string> AddedTracked;
        std::unordered_set<std::string> RemovedTracked;
    };
}
