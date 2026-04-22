#include <unvm/json.hxx>
#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <json/json.hxx>

#include <fstream>
#include <iostream>

int unvm::ReadConfigFile(Config &config)
{
    auto data_directory = GetDataDirectory();
    auto path = data_directory / "config.json";

    if (!std::filesystem::exists(path))
    {
        return 0;
    }

    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open config file." << std::endl;
        return 1;
    }

    json::Node json;
    stream >> json;

    if (!(json >> config))
    {
        std::cerr << "failed to parse config file." << std::endl;
        return 1;
    }

    return 0;
}

int unvm::WriteConfigFile(const Config &config)
{
    auto data_directory = GetDataDirectory();
    auto path = data_directory / "config.json";

    if (std::error_code ec; std::filesystem::create_directories(data_directory, ec), ec)
    {
        std::cerr << "failed to create data directory: " << ec.message() << " (" << ec.value() << ")." << std::endl;
        return 1;
    }

    std::ofstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open config file." << std::endl;
        return 1;
    }

    stream << json::Node(config);
    return 0;
}

static bool read_exact_version(
    const std::filesystem::path &path,
    unvm::VersionType &type,
    std::optional<std::string> &version)
{
    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open '" << path.string() << "'." << std::endl;
        return false;
    }

    std::string line;
    std::getline(stream, line);

    type = unvm::VersionType::Exact;
    version = std::move(line);
    return true;
}

static bool read_package_version(
    const std::filesystem::path &path,
    unvm::VersionType &type,
    std::optional<std::string> &version)
{
    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open '" << path.string() << "'." << std::endl;
        return false;
    }

    json::Node node;
    stream >> node;

    if (!node)
    {
        std::cerr << "failed to parse '" << path.string() << "'." << std::endl;
        return false;
    }

    std::optional<std::string> maybe;
    if (!(node["engines"]["node"] >> maybe))
    {
        std::cerr << "failed to convert engines.node to string." << std::endl;
        return false;
    }

    if (!maybe)
    {
        std::cerr << "no engines.node in package." << std::endl;
        return false;
    }

    type = unvm::VersionType::Package;
    version = *std::move(maybe);
    return true;
}

unvm::VersionType unvm::FindActiveVersion(std::optional<std::string> &version)
{
    const auto current_path = std::filesystem::weakly_canonical(std::filesystem::current_path());

    for (auto parent_path = current_path;;)
    {
        if (auto entry = parent_path / ".unvm"; std::filesystem::exists(entry))
        {
            if (VersionType type; read_exact_version(entry, type, version))
            {
                return type;
            }
        }

        if (auto entry = parent_path / "package.json"; std::filesystem::exists(entry))
        {
            // TODO: if package has no version, continue upwards, until either other config file or package is found.
            // TODO: if parent package is found, and workspaces entry contains current path file tree, if that package has a version, return that, or else return default

            if (VersionType type; read_package_version(entry, type, version))
            {
                return type;
            }
        }

        auto next = parent_path.parent_path();
        if (next == parent_path)
        {
            return VersionType::Default;
        }

        parent_path = next;
    }
}

bool unvm::FindVersionFile(std::filesystem::path &path)
{
    for (auto parent_path = std::filesystem::weakly_canonical(std::filesystem::current_path());;)
    {
        if (auto entry = parent_path / ".unvm"; exists(entry))
        {
            path = std::move(entry);
            return true;
        }

        auto next = parent_path.parent_path();
        if (next == parent_path)
        {
            return false;
        }

        parent_path = next;
    }
}

int unvm::ReadVersionFile(std::optional<std::string> &version)
{
    std::filesystem::path path;
    if (!FindVersionFile(path))
    {
        version = std::nullopt;
        return 0;
    }

    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open file '" << path.string() << "'." << std::endl;
        return 1;
    }

    std::string str;
    std::getline(stream, str);

    version = std::move(str);
    return 0;
}

int unvm::WriteVersionFile(const std::string &version)
{
    std::ofstream stream(".unvm");
    if (!stream)
    {
        std::cerr << "failed to open file '.unvm'." << std::endl;
        return 1;
    }

    stream << version << std::endl;
    return 0;
}

int unvm::RemoveVersionFile()
{
    std::filesystem::path path;
    if (!FindVersionFile(path))
    {
        return 0;
    }

    if (std::error_code ec; std::filesystem::remove(path, ec), ec)
    {
        std::cerr
                << "failed to delete file '"
                << path.string()
                << "': "
                << ec.message()
                << " ("
                << ec.value()
                << ")."
                << std::endl;
        return 1;
    }

    return 0;
}
