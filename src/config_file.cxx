#include <unvm/config.hxx>
#include <unvm/json.hxx>
#include <unvm/util.hxx>

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
    if (!config.Dirty)
    {
        return 0;
    }

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
