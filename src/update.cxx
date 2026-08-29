#include <unvm/config.hxx>
#include <unvm/lock.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <toolkit/string.hxx>

toolkit::result<> unvm::Update(
    Config &config,
    const http::client &client,
    std::string_view tag,
    const VersionEntry &entry,
    const VersionEntry *pre)
{
    const auto data_directory = GetDataDirectory();
    const auto lock_path = data_directory / (entry.Version + ".lock");

    TryAcquire lock(lock_path, true, "install");
    if (!lock.Primary())
    {
        if (auto res = ReloadConfigFile(config); !res)
        {
            return res;
        }
    }

    (void) lock;

    if (auto res = Install(config, client, tag, entry, true); !res)
    {
        return res;
    }

    if (pre && pre->Version != entry.Version)
    {
        const auto pre_lock_path = data_directory / (pre->Version + ".lock");

        TryAcquire pre_lock(pre_lock_path, true, "remove");
        if (!pre_lock.Primary())
        {
            if (auto res = ReloadConfigFile(config); !res)
            {
                return res;
            }
        }

        (void) pre_lock;

        if (const auto it = config.Installed.find(pre->Version); it != config.Installed.end() && it->second)
        {
            if (config.Default == pre->Version)
            {
                config.Default = entry.Version;
                config.UpdatedDefault = true;
            }

            if (auto res = Remove(config, tag, *pre); !res)
            {
                return res;
            }
        }
    }

    return WriteConfigFile(config);
}

toolkit::result<> unvm::Update(
    Config &config,
    const http::client &client)
{
    VersionTable table;
    if (auto res = LoadVersionTable(client, true) >> table; !res)
    {
        return toolkit::make_error("failed to load version table: {}", res.error());
    }

    FilterVersionTable(config, table, true);

    auto installed = table;
    FilterVersionTable(config, installed, true, true);

    for (auto &tag : config.Tracked)
    {
        auto *entry = FindEffectiveVersion(table, tag);
        if (!entry)
        {
            return toolkit::make_error("no version for tracked tag '{}'.", tag);
        }

        auto *pre = FindEffectiveVersion(installed, entry->Version);

        if (auto res = Update(config, client, tag, *entry, pre); !res)
        {
            return res;
        }
    }

    return {};
}

toolkit::result<> unvm::Update(
    Config &config,
    const http::client &client,
    std::string_view tag)
{
    const auto tag_str = toolkit::lowercase(tag);

    if (!config.Tracked.contains(tag_str))
    {
        return toolkit::make_error("tag '{}' is not being tracked.", tag);
    }

    VersionTable table;
    if (auto res = LoadVersionTable(client, true) >> table; !res)
    {
        return toolkit::make_error("failed to load version table: {}", res.error());
    }

    FilterVersionTable(config, table, true);

    auto installed = table;
    FilterVersionTable(config, installed, true, true);

    auto *entry = FindEffectiveVersion(table, tag_str);
    if (!entry)
    {
        return toolkit::make_error("no version for tracked tag '{}'.", tag);
    }

    auto *pre = FindEffectiveVersion(installed, entry->Version);

    return Update(config, client, tag, *entry, pre);
}
