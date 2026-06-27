#include <unvm/lock.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

toolkit::result<> unvm::Remove(Config &config, http::HttpClient &client, const std::string_view version)
{
    VersionTable table;
    if (auto res = LoadVersionTable(client, table, false); !res)
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
        std::cout << "version '" << version << "' is not installed." << std::endl;
        return {};
    }

    const auto data_directory = GetDataDirectory();
    const auto lock_path = data_directory / (entry->Version + ".lock");

    TryAcquire lock(lock_path, false, "remove");
    if (!lock)
    {
        if (lock.Message() == "remove")
        {
            std::cout << "version '" << version << "' is already being removed by another process." << std::endl;
            return {};
        }

        lock = TryAcquire(lock_path, true, "remove");

        if (auto res = ReloadConfigFile(config); !res)
        {
            return res;
        }
    }

    (void) lock;

    std::filesystem::remove_all(data_directory / entry->Version);

    config.Installed.erase(entry->Version);
    config.RemovedVersions.insert(entry->Version);
    return {};
}
