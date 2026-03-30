#include <unvm/unvm.hxx>

#include <iostream>

int unvm::Remove(Config &config, std::string_view version, const VersionEntry &entry)
{
    if (config.Active.has_value() && config.Active.value() == entry.Version)
    {
        std::cerr << "version '" << version << "' is still in use." << std::endl;
        return 1;
    }

    std::filesystem::remove_all(config.InstallDirectory / entry.Version);

    config.Installed.erase(entry.Version);
    return 0;
}

int unvm::Remove(Config &config, http::HttpClient &client, std::string_view version)
{
    unvm::VersionTable table;
    if (const auto error = unvm::LoadVersionTable(client, table, false))
    {
        return error;
    }

    const auto entry_ptr = unvm::FindEffectiveVersion(table, version);

    if (!entry_ptr || !config.Installed.contains(entry_ptr->Version))
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 0;
    }

    return Remove(config, version, *entry_ptr);
}
