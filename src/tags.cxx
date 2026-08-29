#include <unvm/table.hxx>
#include <unvm/unvm.hxx>

#include <toolkit/string.hxx>

#include <iostream>
#include <ranges>

toolkit::result<> unvm::Tags(const Config &config, const http::client &client, const bool available, const bool flat)
{
    std::vector<std::pair<std::string, std::string>> tags;

    VersionTable table;
    if (auto res = LoadVersionTable(client, available) >> table; !res)
    {
        return toolkit::make_error("failed to load version table: {}", res.error());
    }

    if (available)
    {
        std::unordered_set<std::string> keys;
        for (auto &entry : table)
        {
            if (!keys.contains("latest"))
            {
                tags.emplace_back("latest", entry.Version);
                keys.insert("latest");
            }

            if (entry.LTS && !keys.contains(*entry.LTS))
            {
                tags.emplace_back(*entry.LTS, entry.Version);
                keys.insert(*entry.LTS);
            }
        }
    }
    else
    {
        std::unordered_set<std::string> keys;
        for (auto &entry : table)
        {
            if (!keys.contains("latest") && config.Tracked.contains("latest"))
            {
                tags.emplace_back("latest", entry.Version);
                keys.insert("latest");
            }

            if (entry.LTS && !keys.contains(*entry.LTS) && config.Tracked.contains(toolkit::lowercase(*entry.LTS)))
            {
                tags.emplace_back(*entry.LTS, entry.Version);
                keys.insert(*entry.LTS);
            }
        }
    }

    if (flat)
    {
        for (auto &tag : tags | std::views::keys)
        {
            std::cout << tag << ' ' << toolkit::lowercase(tag) << ' ';
        }

        return {};
    }

    Table out(
        {
            { "", false },
            { "Tag", true },
            { "Latest", true },
            { "Update", true },
        }
    );

    for (auto &[tag, latest] : tags)
    {
        const auto tracked = config.Tracked.contains(toolkit::lowercase(tag));
        const auto update = tracked && !config.Installed.contains(latest);

        if (available)
        {
            out << (tracked ? "*" : "");
        }
        else
        {
            out << "";
        }

        out << tag << latest << (update ? "*" : "");
    }

    if (out.Empty())
    {
        std::cerr << "no elements to list." << std::endl;
        return {};
    }

    std::cout << out;
    return {};
}
