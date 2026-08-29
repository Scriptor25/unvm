#include <unvm/lock.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

toolkit::result<> unvm::Remove(
    Config &config,
    const std::string_view version,
    const VersionEntry &entry)
{
    const auto it = config.Installed.find(entry.Version);
    if (it == config.Installed.end())
    {
        return {};
    }

    const auto data_directory = GetDataDirectory();
    const auto entry_directory = data_directory / entry.Version;

    if (std::error_code error; std::filesystem::remove_all(entry_directory, error), error)
    {
        return toolkit::make_error(
            "failed to remove version '{}' entry directory '{}': {} ({})",
            version,
            entry_directory.string(),
            error.message(),
            error.value());
    }

    if (config.Default == entry.Version)
    {
        config.Default = std::nullopt;
        config.UpdatedDefault = true;
    }

    config.Installed.erase(it);
    config.RemovedVersions.insert(entry.Version);
    return {};
}

toolkit::result<> unvm::Remove(Config &config, const http::client &client, const std::string_view version)
{
    VersionTable table;
    if (auto res = LoadVersionTable(client, false) >> table; !res)
    {
        return res;
    }

    FilterVersionTable(config, table, true, true);

    const VersionEntry *entry;
    if (auto res = FindVersionEntry(table, version) >> entry; !res)
    {
        return res;
    }

    if (!entry)
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return {};
    }

    const auto data_directory = GetDataDirectory();
    const auto lock_path = data_directory / (entry->Version + ".lock");

    TryAcquire lock(lock_path, false, "remove");
    if (!lock)
    {
        if (lock.Message() == "remove")
        {
            std::cerr << "version '" << version << "' is already being removed by another process." << std::endl;
            return {};
        }

        lock = TryAcquire(lock_path, true, "remove");

        if (auto res = ReloadConfigFile(config); !res)
        {
            return res;
        }
    }

    (void) lock;

    if (auto res = Remove(config, version, *entry); !res)
    {
        return res;
    }

    return WriteConfigFile(config);
}
