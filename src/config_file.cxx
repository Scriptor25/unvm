#include <unvm/config.hxx>
#include <unvm/json.hxx>
#include <unvm/util.hxx>

#include <fstream>
#include <iostream>

toolkit::result<> unvm::ReadConfigFile(Config &config)
{
    auto data_directory = GetDataDirectory();
    auto path = data_directory / "config.json";

    if (!std::filesystem::exists(path))
    {
        return {};
    }

    std::ifstream stream(path);
    if (!stream)
    {
        return toolkit::make_error("failed to open config file.");
    }

    json::Node json;
    stream >> json;

    if (!(json >> config))
    {
        return toolkit::make_error("failed to parse config file.");
    }

    return {};
}

toolkit::result<> unvm::WriteConfigFile(const Config &config)
{
    if (!config.Dirty)
    {
        return {};
    }

    auto data_directory = GetDataDirectory();
    auto path = data_directory / "config.json";

    if (std::error_code ec; std::filesystem::create_directories(data_directory, ec), ec)
    {
        return toolkit::make_error("failed to create data directory: {} ({}).", ec.message(), ec.value());
    }

    std::ofstream stream(path);
    if (!stream)
    {
        return toolkit::make_error("failed to open config file.");
    }

    stream << json::Node(config);
    return {};
}
