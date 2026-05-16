#include <toolkit/string.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

const unvm::VersionEntry *unvm::FindEffectiveVersion(const VersionTable &table, const std::string_view version)
{
    // latest
    if (version == "latest")
    {
        if (!table.empty())
        {
            return &table.front();
        }
    }
    // latest lts
    else if (version == "lts")
    {
        for (auto &entry : table)
        {
            if (entry.Lts)
            {
                return &entry;
            }
        }
    }
    // version by pattern
    else if (isdigit(version.front()))
    {
        switch (const auto segments = toolkit::split(version, '.'); segments.size())
        {
        case 1:
        case 2:
        {
            const auto pattern = 'v' + std::string(version) + '.';
            for (auto &entry : table)
            {
                if (entry.Version.starts_with(pattern))
                {
                    return &entry;
                }
            }
            break;
        }

        case 3:
        {
            const auto pattern = 'v' + std::string(version);
            for (auto &entry : table)
            {
                if (entry.Version == pattern)
                {
                    return &entry;
                }
            }
            break;
        }

        default:
            break;
        }
    }
    else if (version.front() == 'v')
    {
        switch (const auto segments = toolkit::split(version.substr(1), '.'); segments.size())
        {
        case 1:
        case 2:
        {
            const auto pattern = 'v' + std::string(version) + '.';
            for (auto &entry : table)
            {
                if (entry.Version.starts_with(pattern))
                {
                    return &entry;
                }
            }
            break;
        }

        case 3:
        {
            for (auto &entry : table)
            {
                if (entry.Version == version)
                {
                    return &entry;
                }
            }
            break;
        }

        default:
            break;
        }
    }
    // lts by name
    else
    {
        const auto name = toolkit::lowercase(std::string(version));
        for (auto &entry : table)
        {
            if (entry.Lts && toolkit::lowercase(*entry.Lts) == name)
            {
                return &entry;
            }
        }
    }

    return nullptr;
}
