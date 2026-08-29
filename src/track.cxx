#include <unvm/unvm.hxx>

#include <toolkit/result.hxx>
#include <toolkit/string.hxx>

#include <iostream>

toolkit::result<> unvm::Track(
    Config &config,
    const http::client &client,
    std::string_view tag)
{
    const auto tag_str = toolkit::lowercase(tag);

    if (config.Tracked.contains(tag_str))
    {
        std::cerr << "tag '" << tag << "' is already being tracked." << std::endl;
        return {};
    }

    VersionTable table;
    if (auto res = LoadVersionTable(client, true) >> table; !res)
    {
        return res;
    }

    FilterVersionTable(config, table, true);

    auto valid = false;
    if (tag_str == "latest")
    {
        valid = true;
    }
    else
    {
        for (auto &entry : table)
        {
            if (entry.LTS && toolkit::lowercase(*entry.LTS) == tag_str)
            {
                valid = true;
                break;
            }
        }
    }

    if (!valid)
    {
        return toolkit::make_error("tag '{}' is not a valid tag.", tag);
    }

    config.Tracked.insert(tag_str);
    config.AddedTracked.insert(tag_str);
    return {};
}

toolkit::result<> unvm::Untrack(
    Config &config,
    const http::client &client,
    const std::string_view tag)
{
    const auto tag_str = toolkit::lowercase(tag);

    const auto it = config.Tracked.find(tag_str);
    if (it == config.Tracked.end())
    {
        std::cerr << "tag '" << tag << "' is not being tracked." << std::endl;
        return {};
    }

    VersionTable table;
    if (auto res = LoadVersionTable(client, false) >> table; !res)
    {
        return toolkit::make_error("failed to load version table: {}", res.error());
    }

    FilterVersionTable(config, table, true, true);

    if (auto *entry = FindEffectiveVersion(table, tag_str))
    {
        config.Installed[entry->Version] = false;
        config.UpdatedVersions.insert(entry->Version);
    }

    config.Tracked.erase(it);
    config.RemovedTracked.insert(tag_str);
    return {};
}
