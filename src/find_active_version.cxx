#include <unvm/json.hxx>
#include <unvm/util.hxx>

#include <fstream>
#include <iostream>

static int read_exact_version(
    const std::filesystem::path &path,
    std::optional<std::string> &version)
{
    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open '" << path.string() << "'." << std::endl;
        return 1;
    }

    std::string line;
    std::getline(stream, line);

    version = std::move(line);
    return 0;
}

static int read_package_version(
    const std::filesystem::path &path,
    std::optional<std::string> &version)
{
    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open '" << path.string() << "'." << std::endl;
        return 1;
    }

    json::Node node;
    stream >> node;

    if (!node)
    {
        std::cerr << "failed to parse '" << path.string() << "'." << std::endl;
        return 1;
    }

    std::optional<std::string> maybe;
    if (!(node["engines"]["node"] >> maybe))
    {
        std::cerr << "failed to convert key 'engines.node' in '" << path.string() << "' to string." << std::endl;
        return 1;
    }

    if (maybe)
    {
        version = *std::move(maybe);
    }

    return 0;
}

int unvm::FindActiveVersion(std::optional<std::string> &version, VersionType *type)
{
    const auto current_path = std::filesystem::weakly_canonical(std::filesystem::current_path());

    for (auto parent_path = current_path;;)
    {
        if (auto entry = parent_path / ".unvm"; std::filesystem::exists(entry))
        {
            if (const auto error = read_exact_version(entry, version))
            {
                return error;
            }

            if (version)
            {
                if (type)
                {
                    *type = VersionType::Exact;
                }
                return 0;
            }
        }

        if (auto entry = parent_path / "package.json"; std::filesystem::exists(entry))
        {
            // TODO: if package has no version, continue upwards, until either other config file or package is found.
            // TODO: if parent package is found, and workspaces entry contains current path file tree, if that package has a version, return that, or else return default

            if (const auto error = read_package_version(entry, version))
            {
                return error;
            }

            if (version)
            {
                if (type)
                {
                    *type = VersionType::Package;
                }
                return 0;
            }
        }

        auto next = parent_path.parent_path();
        if (next == parent_path)
        {
            if (type)
            {
                *type = VersionType::Default;
            }
            return 0;
        }

        parent_path = next;
    }
}
