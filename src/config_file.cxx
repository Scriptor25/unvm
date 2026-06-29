#include <unvm/config.hxx>
#include <unvm/json.hxx>
#include <unvm/lock.hxx>
#include <unvm/util.hxx>

#include <fstream>

toolkit::result<> unvm::ReadConfigFile(Config &config)
{
    auto data_directory = GetDataDirectory();

    if (std::error_code ec; std::filesystem::create_directories(data_directory, ec), ec)
    {
        return toolkit::make_error("failed to create data directory: {} ({}).", ec.message(), ec.value());
    }

    auto path = data_directory / "config.json";
    auto lock_path = data_directory / "config.lock";

    FileLock lock;
    if (auto res = FileLock::Lock(lock_path) >> lock; !res)
    {
        return res;
    }

    if (!std::filesystem::exists(path))
    {
        config = {};
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

toolkit::result<> unvm::WriteConfigFile(Config &config)
{
    if (!config.DefaultDirty
        && config.AddedVersions.empty()
        && config.RemovedVersions.empty()
        && config.AddedFingerprints.empty()
        && config.RemovedFingerprints.empty())
    {
        return {};
    }

    auto data_directory = GetDataDirectory();

    if (std::error_code ec; std::filesystem::create_directories(data_directory, ec), ec)
    {
        return toolkit::make_error("failed to create data directory: {} ({}).", ec.message(), ec.value());
    }

    auto path = data_directory / "config.json";
    auto temp_path = data_directory / "config.temp";
    auto lock_path = data_directory / "config.lock";

    FileLock lock;
    if (auto res = FileLock::Lock(lock_path) >> lock; !res)
    {
        return res;
    }

    Config merged;
    if (std::filesystem::exists(path))
    {
        std::ifstream stream(path);
        if (!stream)
        {
            return toolkit::make_error("failed to open config file.");
        }

        json::Node json;
        stream >> json;

        if (!(json >> merged))
        {
            return toolkit::make_error("failed to parse config file.");
        }
    }

    if (config.DefaultDirty)
    {
        merged.Default = config.Default;
    }

    for (auto &version : config.AddedVersions)
    {
        merged.Installed.insert(version);
    }

    for (auto &version : config.RemovedVersions)
    {
        merged.Installed.erase(version);
    }

    for (auto &version : config.AddedFingerprints)
    {
        merged.Fingerprints.insert(version);
    }

    for (auto &version : config.RemovedFingerprints)
    {
        merged.Fingerprints.erase(version);
    }

    std::ofstream stream(temp_path);
    if (!stream)
    {
        return toolkit::make_error("failed to open config file.");
    }

    stream << json::Node(merged);
    stream.close();

    if (std::error_code ec; std::filesystem::remove(path), ec)
    {
        return toolkit::make_error("failed to remove config file: {} ({}).", ec.message(), ec.value());
    }

    if (std::error_code ec; std::filesystem::rename(temp_path, path), ec)
    {
        return toolkit::make_error("failed to rename config file: {} ({}).", ec.message(), ec.value());
    }

    config.DefaultDirty = false;
    config.AddedVersions.clear();
    config.RemovedVersions.clear();
    config.AddedFingerprints.clear();
    config.RemovedFingerprints.clear();
    return {};
}

toolkit::result<> unvm::ReloadConfigFile(Config &config)
{
    if (auto res = WriteConfigFile(config); !res)
    {
        return res;
    }

    return ReadConfigFile(config);
}
