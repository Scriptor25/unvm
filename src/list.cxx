#include <unvm/table.hxx>
#include <unvm/unvm.hxx>

#include <toolkit/string.hxx>

#include <iostream>

toolkit::result<> unvm::List(
    const Config &config,
    http::HttpClient &client,
    const bool available,
    const bool flat,
    const bool details)
{
    VersionTable table;
    if (auto res = LoadVersionTable(client, table, available); !res)
    {
        return res;
    }

    FilterVersionTable(config, table, true, !available);

    if (flat)
    {
        std::unordered_set<std::string> versions;

        for (auto &entry : table)
        {
            versions.insert(entry.Version);
            versions.insert(entry.Version.substr(1));

            if (entry.LTS)
            {
                versions.insert(*entry.LTS);
                versions.insert(toolkit::lowercase(*entry.LTS));
            }
        }

        for (const auto &version : versions)
        {
            std::cout << version << ' ';
        }

        return {};
    }

    Table out;

    if (details)
    {
        out.Init(
            {
                { "Installed", true },
                { "Security", true },
                { "LTS", true },
                { "Version", true },
                { "NPM", true },
                { "V8", true },
                { "UV", true },
                { "ZLib", true },
                { "OpenSSL", true },
                { "Date", true },
                { "Modules", false },
            });

        for (auto &entry : table)
        {
            out
                    << (config.Active == entry.Version ? "yes" : "")
                    << (entry.Security ? "yes" : "")
                    << entry.LTS.value_or(std::string())
                    << entry.Version
                    << entry.NPM.value_or(std::string())
                    << entry.V8
                    << entry.UV.value_or(std::string())
                    << entry.ZLib.value_or(std::string())
                    << entry.OpenSSL.value_or(std::string())
                    << entry.Date
                    << std::to_string(entry.Modules);
        }
    }
    else
    {
        out.Init(
            {
                { {}, false },
                { "LTS", true },
                { "Version", true },
                { "NPM", true },
                { "Date", true },
            });

        std::unordered_set<std::string> major_versions;

        for (auto &entry : table)
        {
            auto segments = toolkit::split(entry.Version, '.');
            if (available && major_versions.contains(segments.front()))
            {
                continue;
            }

            major_versions.insert(segments.front());

            out
                    << (config.Active == entry.Version ? "*" : "")
                    << entry.LTS.value_or(std::string())
                    << entry.Version
                    << entry.NPM.value_or(std::string())
                    << entry.Date;
        }
    }

    if (out.Empty())
    {
        std::cout << "no elements to list." << std::endl;
        return {};
    }

    std::cout << out;
    return {};
}
