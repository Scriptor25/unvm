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

    const auto entry = FindEffectiveVersion(table, version);
    if (!entry || !config.Installed.contains(entry->Version))
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return {};
    }

    const auto data_directory = GetDataDirectory();
    std::filesystem::remove_all(data_directory / entry->Version);

    config.Installed.erase(entry->Version);
    config.Dirty = true;
    return {};
}
