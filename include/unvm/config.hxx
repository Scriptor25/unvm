#pragma once

#include <toolkit/result.hxx>

#include <optional>
#include <string>
#include <unordered_set>

namespace unvm
{
    struct Config
    {
        std::optional<std::string> Default;
        std::unordered_set<std::string> Installed;
        std::unordered_set<std::string> Fingerprints;

        std::optional<std::string> Active;
        std::optional<std::string> Detected;

        bool UpdatedDefault;
        std::unordered_set<std::string> AddedVersions;
        std::unordered_set<std::string> RemovedVersions;
        std::unordered_set<std::string> AddedFingerprints;
        std::unordered_set<std::string> RemovedFingerprints;
    };

    /**
     * Merge two in-memory configs. Read the changes from src and apply them to dst.
     *
     * @param dst
     * @param src
     */
    void MergeConfig(Config &dst, const Config &src);

    /**
     * Read the config atomically from disk to the given memory reference.
     *
     * @param config
     * @return
     */
    [[nodiscard]] toolkit::result<> ReadConfigFile(Config &config);
    /**
     * Write the config atomically from the given memory reference to disk. This first reads the current config from
     * disk, then merges it with the new config and writes it back to disk in one atomic operation.
     *
     * @param config
     * @return
     */
    [[nodiscard]] toolkit::result<> WriteConfigFile(Config &config);
    /**
     * Read the config atomically from disk to the given memory references, while preserving changes to the
     * in-memory config.
     *
     * @param config
     * @return
     */
    [[nodiscard]] toolkit::result<> ReloadConfigFile(Config &config);

}
