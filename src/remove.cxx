#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

int unvm::Remove(Config &config, http::HttpClient &client, const std::string_view version)
{
    VersionTable table;
    if (const auto error = LoadVersionTable(client, table, false))
    {
        return error;
    }

    const auto entry = FindEffectiveVersion(table, version);
    if (!entry || !config.Installed.contains(entry->Version))
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 0;
    }

    const auto data_directory = GetDataDirectory();
    std::filesystem::remove_all(data_directory / entry->Version);

    config.Installed.erase(entry->Version);
    return 0;
}
