#include <unvm/lock.hxx>
#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

toolkit::result<> unvm::Update(Config &config, http::HttpClient &client)
{
    VersionTable table;
    if (auto res = LoadVersionTable(client, table, true); !res)
    {
        return res;
    }

    std::unordered_set<std::string> updated;

    for (auto &[name, version] : config.Tracked)
    {
        const VersionEntry *entry;
        if (auto res = FindVersionEntry(table, version) >> entry; !res)
        {
            return res;
        }

        if (!entry)
        {
            return toolkit::make_error("failed to match tracked version range '{}' ('{}').", name, version);
        }

        const auto data_directory = GetDataDirectory();
        const auto lock_path = data_directory / (entry->Version + ".lock");

        TryAcquire lock(lock_path, false, "install");
        if (!lock)
        {
            if (lock.Message() == "install")
            {
                std::cout << "version '" << version << "' is already being installed by another process." << std::endl;
                continue;
            }

            lock = TryAcquire(lock_path, true, "install");

            if (auto res = ReloadConfigFile(config); !res)
            {
                return res;
            }
        }

        (void) lock;

        if (config.Installed.contains(entry->Version))
        {
            continue;
        }

        if (auto res = Install(config, client, version, *entry); !res)
        {
            return res;
        }

        updated.insert(name);
    }

    if (updated.empty())
    {
        std::cout << "everything up-to-date." << std::endl;
    }
    else
    {
        std::cout << "updated ";
        for (auto it = updated.begin(); it != updated.end(); ++it)
        {
            if (it != updated.begin())
            {
                std::cout << ", ";
            }

            std::cout << '\'' << *it << '\'';
        }
        std::cout << '.' << std::endl;
    }

    return {};
}

toolkit::result<> unvm::Update(Config &config, http::HttpClient &client, std::string_view name)
{
    const auto it = config.Tracked.find(std::string(name));
    if (it == config.Tracked.end())
    {
        return toolkit::make_error("version track '{}' does not exist.", name);
    }

    VersionTable table;
    if (auto res = LoadVersionTable(client, table, true); !res)
    {
        return res;
    }

    FilterVersionTable(config, table, true);

    auto &version = it->second;

    const VersionEntry *entry;
    if (auto res = FindVersionEntry(table, version) >> entry; !res)
    {
        return res;
    }

    if (!entry)
    {
        return toolkit::make_error("failed to match tracked version range '{}' ('{}').", name, version);
    }

    const auto data_directory = GetDataDirectory();
    const auto lock_path = data_directory / (entry->Version + ".lock");

    TryAcquire lock(lock_path, false, "install");
    if (!lock)
    {
        if (lock.Message() == "install")
        {
            std::cout << "version '" << version << "' is already being installed by another process." << std::endl;
            return {};
        }

        lock = TryAcquire(lock_path, true, "install");

        if (auto res = ReloadConfigFile(config); !res)
        {
            return res;
        }
    }

    (void) lock;

    if (config.Installed.contains(entry->Version))
    {
        std::cout << "everything up-to-date." << std::endl;
        return {};
    }

    if (auto res = Install(config, client, version, *entry); !res)
    {
        return res;
    }

    std::cout << "updated '" << name << "'." << std::endl;
    return {};
}
