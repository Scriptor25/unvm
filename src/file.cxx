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

    std::ifstream stream(data_directory);
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

bool unvm::FindVersionFile(std::filesystem::path &path)
{
    for (auto parent_path = std::filesystem::weakly_canonical(std::filesystem::current_path());;)
    {
        if (std::filesystem::exists(parent_path / ".unvm"))
        {
            path = parent_path / ".unvm";
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

    if (std::error_code ec; std::filesystem::remove(path, ec), ec)
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
