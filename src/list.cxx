#include <unvm/table.hxx>
#include <unvm/unvm.hxx>

#include <toolkit/string.hxx>

#include <iostream>

toolkit::result<> unvm::List(
    const Config &config,
    http::HttpClient &client,
    const bool available,
    const bool flat)
{
    VersionTable table;
    if (auto res = LoadVersionTable(client, table, available); !res)
    {
        return res;
    }

    if (flat)
    {
        std::unordered_set<std::string> versions;

        for (auto &entry : table)
        {
            if (available || config.Installed.contains(entry.Version))
            {
                versions.insert(entry.Version);

                if (entry.Version.front() == 'v')
                {
                    versions.insert(entry.Version.substr(1));
                }

                if (entry.Lts && !versions.contains(*entry.Lts))
                {
                    versions.insert(*entry.Lts);
                    versions.insert(toolkit::lowercase(*entry.Lts));
                }
            }
        }

        for (const auto &version : versions)
        {
            std::cout << version << ' ';
        }

        return {};
    }

    Table out(
        {
            { {}, false },
            { "Lts", true },
            { "Version", true },
            { "Npm", true },
            { "Date", true },
            { "Modules", false }
        });

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

    std::cout << out;
    return {};
}
