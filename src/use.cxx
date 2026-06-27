#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <iostream>

toolkit::result<> unvm::Use(Config &config, http::HttpClient &client, const std::string_view version, const bool local)
{
    std::optional<std::string> maybe_active;
    VersionType type;

    if (local)
    {
        if (auto res = FindActiveVersion(config.Default, &type) >> maybe_active; !res)
        {
            return toolkit::make_error("failed to find active version: {}", res.error());
        }
    }
    else if (config.Default)
    {
        maybe_active = *config.Default;
    }

    if (version == "none")
    {
        if (!maybe_active)
        {
            std::cerr << "node is already not active in the current context." << std::endl;
            return {};
        }

        if (!local)
        {
            config.Default = std::nullopt;
            config.DefaultDirty = true;
            return {};
        }

        if (type == VersionType::Package)
        {
            return toolkit::make_error("cannot use no version in the current context.");
        }

        if (type != VersionType::Exact)
        {
            return {};
        }

        if (auto res = RemoveVersionFile(); !res)
        {
            return toolkit::make_error("failed to remove version file: {}", res.error());
        }

        return {};
    }

    VersionTable table;
    if (auto res = LoadVersionTable(client, table, false); !res)
    {
        return res;
    }

    FilterVersionTable(config, table, true, true);

    const VersionEntry *entry;
    if (auto res = FindVersionEntry(table, version) >> entry; !res)
    {
        return res;
    }

    if (!entry)
    {
        return toolkit::make_error("version '{}' is not installed.", version);
    }

    if (maybe_active && *maybe_active == entry->Version)
    {
        std::cerr << "version '" << version << "' is already active in the current context." << std::endl;
        return {};
    }

    if (!local)
    {
        config.Default = entry->Version;
        config.DefaultDirty = true;
        return {};
    }

    if (auto res = WriteVersionFile(entry->Version); !res)
    {
        return toolkit::make_error("failed to write version file: {}", res.error());
    }

    return {};
}
