#include <unvm/table.hxx>
#include <unvm/unvm.hxx>

#include <iostream>

int unvm::List(Config &config, http::HttpClient &client, const bool available)
{
    Table out(
        {
            { "Lts", true },
            { "Version", true },
            { "Npm", true },
            { "Date", true },
            { "Modules", false }
        });

    VersionTable table;
    if (const auto error = LoadVersionTable(client, table, available))
        return error;

    for (auto &entry : table)
        if (available || config.Installed.contains(entry.Version))
            out
                << entry.Lts.value_or({})
                << entry.Version
                << entry.Npm.value_or({})
                << entry.Date
                << entry.Modules.value_or({});

    if (out.Empty())
    {
        std::cerr << "no elements to list." << std::endl;
        return 0;
    }

    std::cerr << out;
    return 0;
}
