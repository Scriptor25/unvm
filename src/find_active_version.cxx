#include <unvm/json.hxx>
#include <unvm/util.hxx>

#include <fstream>

[[nodiscard]] static toolkit::result<std::string> read_exact_version(const std::filesystem::path &path)
{
    std::ifstream stream(path);
    if (!stream)
    {
        return toolkit::make_error("failed to open '{}'.", path.string());
    }

    std::string line;
    std::getline(stream, line);

    return line;
}

[[nodiscard]] static toolkit::result<std::optional<std::string>> read_package_version(const std::filesystem::path &path)
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

    return maybe;
}

toolkit::result<std::optional<std::string>> unvm::FindActiveVersion(
    const std::optional<std::string> &def,
    VersionType *type)
{
    bool has_package_json{};

    for (auto parent_path = std::filesystem::weakly_canonical(std::filesystem::current_path());;)
    {
        if (!has_package_json)
        {
            if (auto entry = parent_path / ".unvm"; std::filesystem::exists(entry))
            {
                std::string line;
                if (auto res = read_exact_version(entry) >> line; !res)
                {
                    return res;
                }

                if (type)
                {
                    *type = VersionType::Exact;
                }

                return { std::move(line) };
            }
        }

        if (auto entry = parent_path / "package.json"; std::filesystem::exists(entry))
        {
            std::optional<std::string> line;
            if (auto res = read_package_version(entry) >> line; !res)
            {
                return res;
            }

            if (line)
            {
                if (type)
                {
                    *type = VersionType::Package;
                }

                return line;
            }

            has_package_json = true;
        }

        auto next = parent_path.parent_path();
        if (next == parent_path)
        {
            if (type)
            {
                *type = has_package_json ? VersionType::Package : VersionType::Default;
            }

            return def;
        }

        parent_path = next;
    }
}
