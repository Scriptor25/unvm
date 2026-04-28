#include <unvm/table.hxx>
#include <unvm/unvm.hxx>

#include <iostream>

toolkit::result<> unvm::List(const Config &config, http::HttpClient &client, const bool available)
{
    Table out(
        {
            { {}, false },
            { "Lts", true },
            { "Version", true },
            { "Npm", true },
            { "Date", true },
            { "Modules", false }
        });

    VersionTable table;
    if (auto res = LoadVersionTable(client, table, available); !res)
    {
        return res;
    }

    for (auto &entry : table)
    {
        if (available || config.Installed.contains(entry.Version))
        {
            out
                    << (config.Active == entry.Version ? "*" : "")
                    << entry.Lts.value_or(std::string())
                    << entry.Version
                    << entry.Npm.value_or(std::string())
                    << entry.Date
                    << entry.Modules.value_or(std::string());
        }
    }

    if (out.Empty())
    {
        std::cerr << "no elements to list." << std::endl;
        return {};
    }

    std::cerr << out;
    return {};
}
