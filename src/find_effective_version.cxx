#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <toolkit/string.hxx>

const unvm::VersionEntry *unvm::FindEffectiveVersion(const VersionTable &table, const std::string_view version)
{
    // latest
    if (version == "latest")
    {
        if (!table.empty())
        {
            return &table.front();
        }

        return nullptr;
    }

    // latest lts
    if (version == "lts")
    {
        for (auto &entry : table)
        {
            if (entry.Lts)
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
        if (entry.Lts && toolkit::lowercase(*entry.Lts) == name)
        {
            return &entry;
        }
    }

    return nullptr;
}
