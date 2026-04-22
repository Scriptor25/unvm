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

    if (!exists(path))
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

    if (std::error_code ec; create_directories(data_directory, ec), ec)
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

unvm::FileType unvm::FindVersion(const Config &config, http::HttpClient &client, std::optional<std::string> &version)
{
    auto current_path = weakly_canonical(std::filesystem::current_path());

    for (auto parent_path = current_path;;)
    {
        if (auto entry = parent_path / "package.json"; exists(entry))
        {
            std::ifstream stream(entry);
            if (!stream)
            {
                std::cerr << "warning: failed to open '" << entry.string() << "'." << std::endl;
                continue;
            }

            json::Node node;
            stream >> node;

            if (!node)
            {
                std::cerr << "warning: failed to parse '" << entry.string() << "'." << std::endl;
                continue;
            }

            std::optional<std::string> semver;
            if (!(node["engines"]["node"] >> semver))
            {
                std::cerr << "warning: failed to convert engines.node to string." << std::endl;
                continue;
            }

            if (!semver)
            {
                continue;
            }

            auto set = semver::ParseRangeSet(*semver);

            VersionTable table;
            if (const auto error = LoadVersionTable(client, table, false))
            {
            }

            return FileType::Package;
        }
        
        if (auto entry = parent_path / ".unvm"; exists(entry))
        {
            std::ifstream stream(entry);
            if (!stream)
            {
                std::cerr << "warning: failed to open '" << entry.string() << "'." << std::endl;
                continue;
            }

            std::string line;
            std::getline(stream, line);

            version = std::move(line);
            return FileType::Version;
        }

        auto next = parent_path.parent_path();
        if (next == parent_path)
        {
            if (config.Default)
            {
                version = *config.Default;
            }

            return FileType::Default;
        }

        parent_path = next;
    }
}

bool unvm::FindPackageFile(std::filesystem::path &path)
{
    auto current_path = weakly_canonical(std::filesystem::current_path());

    for (auto parent_path = current_path;;)
    {
        if (auto entry = parent_path / "package.json"; exists(entry))
        {
            std::ifstream stream(entry);
            if (!stream)
            {
                continue;
            }
            json::Node node;
            stream >> node;

            std::optional<std::string> maybe_version;
            node["engines"]["node"] >> maybe_version;

            if (!maybe_version)
            {
                continue;
            }

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

bool unvm::FindVersionFile(std::filesystem::path &path)
{
    for (auto parent_path = weakly_canonical(std::filesystem::current_path());;)
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
        std::cerr << "failed to open local version file '" << path.string() << "'." << std::endl;
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
        std::cerr << "failed to open local version file '.unvm'." << std::endl;
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

    if (std::error_code ec; remove(path, ec), ec)
    {
        std::cerr
                << "failed to delete local version file '"
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
