#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

int unvm::Remove(Config &config, std::string_view version, const VersionEntry &entry)
{
    if (auto it = config.Active.find(entry.Version); it != config.Active.end())
    {
        std::cerr << "version '" << version << "' is still active in " << it->second.size() << " contexts:" << std::endl;
        for (auto &context : it->second)
            std::cerr << " - " << context << std::endl;
        return 1;
    }

    auto data_directory = GetDataDirectory();
    std::filesystem::remove_all(data_directory / entry.Version);

    config.Installed.erase(entry.Version);
    return 0;
}

int unvm::Remove(Config &config, http::HttpClient &client, std::string_view version)
{
    unvm::VersionTable table;
    if (const auto error = unvm::LoadVersionTable(client, table, false))
        return error;
    
    for (auto it = table.begin(); it != table.end(); )
    {
        if (!config.Installed.contains(it->Version))
            it = table.erase(it);
        else
            ++it;
    }

    const auto entry_ptr = unvm::FindEffectiveVersion(table, version);
    if (!entry_ptr)
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 0;
    }

    return Remove(config, version, *entry_ptr);
}
