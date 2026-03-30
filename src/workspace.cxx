#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>

#include <json/json.hxx>

#include <fstream>
#include <iostream>

int unvm::Workspace(Config &config, http::Client &client)
{
    auto package_json = std::filesystem::weakly_canonical("./package.json");

    if (!std::filesystem::exists(package_json))
    {
        std::cerr << "no package.json in current directory." << std::endl;
        return 1;
    }

    std::ifstream stream(package_json);

    if (!stream)
    {
        std::cerr << "failed to open package.json." << std::endl;
        return 1;
    }

    json::Node node;
    stream >> node;

    std::optional<std::string> maybe_version;
    node["engines"]["node"] >> maybe_version;

    auto version = maybe_version.value_or("*");
    auto set = semver::ParseRangeSet(version);

    VersionTable table;
    if (const auto error = LoadVersionTable(client, table, false))
    {
        return error;
    }

    for (auto &entry : table)
    {
        if (!config.Installed.contains(entry.Version))
        {
            continue;
        }

        if (!semver::IsInRange(set, entry.Version))
        {
            continue;
        }

        return Use(config, version, entry);
    }

    if (const auto error = LoadVersionTable(client, table, true))
    {
        return error;
    }

    for (auto &entry : table)
    {
        if (!semver::IsInRange(set, entry.Version))
        {
            continue;
        }

        if (const auto error = Install(config, client, version, entry))
        {
            return error;
        }

        if (const auto error = Use(config, version, entry))
        {
            return error;
        }

        return 0;
    }

    std::cerr << "no version match for '" << version << "'." << std::endl;
    return 1;
}
