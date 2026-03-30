#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

static unsigned count_version_segments(std::string_view str)
{
    unsigned segments = 0;

    std::size_t beg = 0, end;
    while ((end = str.find('.', beg)) != std::string_view::npos)
    {
        ++segments;
        beg = end + 1;
    }

    if (beg != str.length())
    {
        ++segments;
    }

    return segments;
}

const unvm::VersionEntry *unvm::FindEffectiveVersion(const VersionTable &table, std::string_view version)
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
            if (entry.Lts.HasValue)
            {
                return &entry;
            }
        }
    }
    // version by pattern
    else if (version.starts_with('v'))
    {
        // vX     -> vX.L.L
        // vX.X   -> vX.X.L
        // vX.X.X -> vX.X.X
        // vX.X.X -> vX.X.X
        // (X = some version)
        // (L = latest version)

        switch (count_version_segments(version))
        {
        case 1:
        case 2:
        {
            const auto pattern = std::string(version) + '.';
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
    }
    // lts by name
    else
    {
        const auto name = Lower(std::string(version));
        for (auto &entry : table)
        {
            if (entry.Lts.HasValue && Lower(entry.Lts.Value) == name)
            {
                return &entry;
            }
        }
    }

    return nullptr;
}
