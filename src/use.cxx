#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

int unvm::Use(Config &config, const std::string_view &version, const VersionEntry &entry, const bool local)
{
    std::optional<std::string> active;
    if (local)
    {
        if (const auto error = ReadVersionFile(active))
        {
            return error;
        }
    }
    else if (config.Default)
    {
        active = *config.Default;
    }

    if (active && *active == entry.Version)
    {
        std::cerr << "version '" << version << "' is already active in the current context." << std::endl;
        return 0;
    }

    if (local)
    {
        if (auto error = WriteVersionFile(entry.Version))
        {
            return error;
        }
    }
    else
    {
        config.Default = entry.Version;
    }

    config.Active[entry.Version].insert(
        local
            ? std::filesystem::weakly_canonical(std::filesystem::current_path()).string()
            : "default");
    return 0;
}

int unvm::Use(Config &config, http::HttpClient &client, const std::string_view version, const bool local)
{
    std::optional<std::string> active;
    if (local)
    {
        if (const auto error = ReadVersionFile(active))
        {
            return error;
        }
    }
    else if (config.Default)
    {
        active = *config.Default;
    }

    if (version == "none")
    {
        if (!active)
        {
            std::cerr << "node is already not active in the current context." << std::endl;
            return 0;
        }

        if (local)
        {
            if (const auto error = RemoveVersionFile())
            {
                return error;
            }
        }
        else
        {
            config.Default = std::nullopt;
        }

        config.Active[*active].erase(
            local
                ? std::filesystem::weakly_canonical(std::filesystem::current_path()).string()
                : "default");
        if (config.Active[*active].empty())
        {
            config.Active.erase(*active);
        }
        return 0;
    }

    VersionTable table;
    if (const auto error = LoadVersionTable(client, table, false))
    {
        return error;
    }

    for (auto it = table.begin(); it != table.end();)
    {
        if (!config.Installed.contains(it->Version))
        {
            it = table.erase(it);
        }
        else
        {
            ++it;
        }
    }

    const auto entry_ptr = FindEffectiveVersion(table, version);
    if (!entry_ptr)
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 1;
    }

    return Use(config, version, *entry_ptr, local);
}
