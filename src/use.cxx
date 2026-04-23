#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

int unvm::Use(Config &config, http::HttpClient &client, const std::string_view version, const bool local)
{
    std::optional<std::string> active;
    VersionType type;

    if (local)
    {
        if (const auto error = FindActiveVersion(active, &type))
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

        if (!local)
        {
            config.Default = std::nullopt;
            config.Dirty = true;
            return 0;
        }

        if (type == VersionType::Package)
        {
            std::cerr << "cannot use no version in the current context." << std::endl;
            return 1;
        }

        if (type != VersionType::Exact)
        {
            return 0;
        }

        if (const auto error = RemoveVersionFile())
        {
            return error;
        }

        return 0;
    }

    VersionTable table;
    if (const auto error = LoadVersionTable(client, table, false))
    {
        return error;
    }

    const auto entry = FindEffectiveVersion(table, version);
    if (!entry || !config.Installed.contains(entry->Version))
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 1;
    }

    if (active && *active == entry->Version)
    {
        std::cerr << "version '" << version << "' is already active in the current context." << std::endl;
        return 0;
    }

    if (!local)
    {
        config.Default = entry->Version;
        config.Dirty = true;
        return 0;
    }

    return WriteVersionFile(entry->Version);
}
