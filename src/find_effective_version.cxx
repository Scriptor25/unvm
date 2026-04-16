#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

const unvm::VersionEntry *unvm::FindEffectiveVersion(const VersionTable &table, std::string_view version)
{
    // latest
    if (version == "latest")
    {
        if (!table.empty())
            return &table.front();
    }
    // latest lts
    else if (version == "lts")
    {
        for (auto &entry : table)
            if (entry.Lts)
                return &entry;
    }
    // version by pattern
    else if (isdigit(version.front()))
    {
        auto segments = Split(std::string(version), '.');
        switch (segments.size())
        {
        case 1:
        case 2:
        {
            const auto pattern = 'v' + std::string(version) + '.';
            for (auto &entry : table)
                if (entry.Version.starts_with(pattern))
                    return &entry;
            break;
        }

        case 3:
        {
            const auto pattern = 'v' + std::string(version);
            for (auto &entry : table)
                if (entry.Version == pattern)
                    return &entry;
            break;
        }

        default:
            break;
        }
    }
    // lts by name
    else
    {
        const auto name = Lower(std::string(version));
        for (auto &entry : table)
            if (entry.Lts && Lower(*entry.Lts) == name)
                return &entry;
    }

    return nullptr;
}
