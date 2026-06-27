#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <toolkit/string.hxx>

const unvm::VersionEntry *unvm::FindEffectiveVersion(
    const VersionTable &table,
    const std::string_view version,
    bool &matched)
{
    // latest
    if (version == "latest")
    {
        matched = true;

        if (!table.empty())
        {
            return &table.front();
        }

        return nullptr;
    }

    // latest lts
    if (version == "lts")
    {
        matched = true;

        for (auto &entry : table)
        {
            if (entry.LTS)
            {
                return &entry;
            }
        }

        return nullptr;
    }

    // version by pattern
    if (isdigit(version.front()))
    {
        switch (const auto segments = toolkit::split(version, '.'); segments.size())
        {
        case 1:
        case 2:
            for (const auto pattern = 'v' + std::string(version) + '.';
                 auto &entry : table)
            {
                if (entry.Version.starts_with(pattern))
                {
                    return &entry;
                }
            }
            break;

        case 3:
            for (const auto pattern = 'v' + std::string(version);
                 auto &entry : table)
            {
                if (entry.Version == pattern)
                {
                    return &entry;
                }
            }
            break;

        default:
            break;
        }

        return nullptr;
    }

    // version by pattern
    if (version.front() == 'v')
    {
        switch (const auto segments = toolkit::split(version, '.'); segments.size())
        {
        case 1:
        case 2:
            for (const auto pattern = std::string(version) + '.';
                 auto &entry : table)
            {
                if (entry.Version.starts_with(pattern))
                {
                    return &entry;
                }
            }
            break;

        case 3:
            for (auto &entry : table)
            {
                if (entry.Version == version)
                {
                    return &entry;
                }
            }
            break;

        default:
            break;
        }

        return nullptr;
    }

    // lts by name
    const auto name = toolkit::lowercase(std::string(version));
    for (auto &entry : table)
    {
        if (entry.LTS && toolkit::lowercase(*entry.LTS) == name)
        {
            return &entry;
        }
    }

    return nullptr;
}

toolkit::result<const unvm::VersionEntry *> unvm::FindVersionEntry(
    const VersionTable &table,
    const std::string_view version)
{
    bool matched{};
    if (auto *effective = FindEffectiveVersion(table, version, matched))
    {
        return effective;
    }

    if (matched)
    {
        return nullptr;
    }

    semver::RangeSet set;
    if (auto res = semver::ParseRangeSet(version) >> set; !res)
    {
        return res;
    }

    for (auto &entry : table)
    {
        bool in_range;
        if (auto res = IsInRange(set, entry.Version) >> in_range; !res)
        {
            return res;
        }

        if (in_range)
        {
            return &entry;
        }
    }

    return nullptr;
}
