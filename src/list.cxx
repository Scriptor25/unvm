#include <unvm/table.hxx>
#include <unvm/unvm.hxx>

#include <iostream>

int unvm::List(Config &config, http::Client &client, const bool available)
{
    Table out(
        {
            { "", true },
            { "Lts", true },
            { "Version", true },
            { "Npm", true },
            { "Date", true },
            { "Modules", false }
        });

    VersionTable table;
    if (const auto error = LoadVersionTable(client, table, available))
    {
        return error;
    }

    for (auto &entry : table)
    {
        if (available || config.Installed.contains(entry.Version))
        {
            out
                    << (config.Active.has_value() && config.Active.value() == entry.Version ? "*" : "")
                    << (entry.Lts.HasValue ? entry.Lts.Value : "")
                    << entry.Version
                    << (entry.Npm.has_value() ? entry.Npm.value() : "")
                    << entry.Date
                    << (entry.Modules.has_value() ? entry.Modules.value() : "");
        }
    }

    if (out.Empty())
    {
        std::cerr << "no elements to list." << std::endl;
        return 0;
    }

    std::cerr << out;
    return 0;
}
