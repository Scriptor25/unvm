#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

int unvm::Use(Config &config, const std::string_view &version, const VersionEntry &entry, bool local)
{
    if (active == entry.Version)
    {
        std::cerr << "version '" << version << "' is already active." << std::endl;
        return 0;
    }

    // TODO: write version to global or local storage
    if (local)
    {
    }
    
    ++config.Active[entry.Version];
    return 0;
}

int unvm::Use(Config &config, http::HttpClient &client, std::string_view version, bool local)
{
    if (version == "none")
    {
        if (!active)
        {
            std::cerr << "node is already inactive." << std::endl;
            return 0;
        }

        // TODO: write version to global or local storage

        --config.Active[active];
        return 0;
    }

    VersionTable table;
    if (const auto error = LoadVersionTable(client, table, false))
        return error;

    for (auto it = table.begin(); it != table.end(); )
    {
        if (!config.Installed.contains(it->Version))
            it = table.erase(it);
        else
            ++it;
    }

    const auto entry_ptr = FindEffectiveVersion(table, version);
    if (!entry_ptr)
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 1;
    }

    return Use(config, version, *entry_ptr, local);
}
