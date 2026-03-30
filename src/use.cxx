#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

int unvm::Use(Config &config, const std::string_view &version, const VersionEntry &entry)
{
    if (config.Active.has_value())
    {
        if (config.Active.value() == entry.Version)
        {
            std::cerr << "version '" << version << "' is already installed and active." << std::endl;
            return 0;
        }

        if (const auto error = RemoveLink(config.ActiveDirectory))
        {
            std::cerr << "failed to un-link current active version '" << config.Active.value() << "'." << std::endl;
            return error;
        }
    }

    if (const auto error = CreateLink(config.ActiveDirectory, config.InstallDirectory / entry.Version))
    {
        std::cerr << "failed to link new active version '" << version << "'." << std::endl;
        return error;
    }

    if (const auto error = AppendUserPath(config.ActiveDirectory))
    {
        std::cerr << "failed to append active directory to user path variable." << std::endl;
        return error;
    }

    config.Active = entry.Version;
    return 0;
}

int unvm::Use(Config &config, http::HttpClient &client, std::string_view version)
{
    if (version == "none")
    {
        if (!config.Active.has_value())
        {
            std::cerr << "node is already inactive." << std::endl;
            return 0;
        }

        if (const auto error = RemoveLink(config.ActiveDirectory))
        {
            std::cerr << "failed to un-link current active version '" << config.Active.value() << "'." << std::endl;
            return error;
        }

        config.Active = std::nullopt;
        return 0;
    }

    VersionTable table;
    if (const auto error = LoadVersionTable(client, table, false))
    {
        return error;
    }

    const auto entry_ptr = FindEffectiveVersion(table, version);

    if (!entry_ptr || !config.Installed.contains(entry_ptr->Version))
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 1;
    }

    return Use(config, version, *entry_ptr);
}
