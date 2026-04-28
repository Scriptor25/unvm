#include <unvm/json.hxx>
#include <unvm/util.hxx>

#include <fstream>
#include <iostream>

static toolkit::result<> read_exact_version(
    const std::filesystem::path &path,
    std::optional<std::string> &version)
{
    std::ifstream stream(path);
    if (!stream)
    {
        return toolkit::make_error("failed to open '{}'.", path.string());
    }

    std::string line;
    std::getline(stream, line);

    version = std::move(line);
    return {};
}

static toolkit::result<> read_package_version(
    const std::filesystem::path &path,
    std::optional<std::string> &version)
{
    std::ifstream stream(path);
    if (!stream)
    {
        return toolkit::make_error("failed to open '{}'.", path.string());
    }

    json::Node node;
    stream >> node;

    if (!node)
    {
        return toolkit::make_error("failed to parse '{}'.", path.string());
    }

    std::optional<std::string> maybe;
    if (!(node["engines"]["node"] >> maybe))
    {
        return toolkit::make_error("failed to convert key 'engines.node' in '{}' to string.", path.string());
    }

    if (maybe)
    {
        version = *std::move(maybe);
    }

    return {};
}

toolkit::result<> unvm::FindActiveVersion(std::optional<std::string> &version, VersionType *type)
{
    const auto current_path = std::filesystem::weakly_canonical(std::filesystem::current_path());

    for (auto parent_path = current_path;;)
    {
        if (auto entry = parent_path / ".unvm"; std::filesystem::exists(entry))
        {
            if (auto res = read_exact_version(entry, version); !res)
            {
                return res;
            }

            if (version)
            {
                if (type)
                {
                    *type = VersionType::Exact;
                }
                return {};
            }
        }

        if (auto entry = parent_path / "package.json"; std::filesystem::exists(entry))
        {
            if (auto res = read_package_version(entry, version); !res)
            {
                return res;
            }

            if (version)
            {
                if (type)
                {
                    *type = VersionType::Package;
                }
                return {};
            }
        }

        auto next = parent_path.parent_path();
        if (next == parent_path)
        {
            if (type)
            {
                *type = VersionType::Default;
            }
            return {};
        }

        parent_path = next;
    }
}
