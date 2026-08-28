#include <unvm/config.hxx>
#include <unvm/json.hxx>
#include <unvm/lock.hxx>
#include <unvm/util.hxx>

#include <fstream>

void unvm::MergeConfig(Config &dst, const Config &src)
{
    if (src.UpdatedDefault)
    {
        dst.Default = src.Default;
    }

    for (auto &key : src.AddedVersions)
    {
        dst.Installed.emplace(key, src.Installed.at(key));
    }

    for (auto &key : src.RemovedVersions)
    {
        dst.Installed.erase(key);
    }

    for (auto &key : src.AddedFingerprints)
    {
        dst.Fingerprints.insert(key);
    }

    for (auto &key : src.RemovedFingerprints)
    {
        dst.Fingerprints.erase(key);
    }

    for (auto &key : src.AddedTracked)
    {
        dst.Tracked.insert(key);
    }

    for (auto &key : src.RemovedTracked)
    {
        dst.Tracked.erase(key);
    }
}

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

    json::node json;
    stream >> json;

    if (!(json >> config))
    {
        return toolkit::make_error("failed to parse config file.");
    }

    return {};
}

toolkit::result<> unvm::WriteConfigFile(Config &config)
{
    if (!config.UpdatedDefault
        && config.AddedVersions.empty()
        && config.RemovedVersions.empty()
        && config.AddedFingerprints.empty()
        && config.RemovedFingerprints.empty()
        && config.AddedTracked.empty()
        && config.RemovedTracked.empty())
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

        json::node json;
        stream >> json;

        if (!(json >> merged))
        {
            return toolkit::make_error("failed to parse config file.");
        }
    }

    MergeConfig(merged, config);

    {
        std::ofstream stream(temp_path);
        if (!stream)
        {
            return toolkit::make_error("failed to open config file.");
        }

        stream << json::node(merged);
        stream.close();
    }

    if (std::error_code ec; std::filesystem::remove(path), ec)
    {
        return toolkit::make_error("failed to remove config file: {} ({}).", ec.message(), ec.value());
    }

    if (std::error_code ec; std::filesystem::rename(temp_path, path), ec)
    {
        return toolkit::make_error("failed to rename config file: {} ({}).", ec.message(), ec.value());
    }

    config.UpdatedDefault = false;
    config.AddedVersions.clear();
    config.RemovedVersions.clear();
    config.AddedFingerprints.clear();
    config.RemovedFingerprints.clear();
    config.AddedTracked.clear();
    config.RemovedTracked.clear();

    return {};
}

toolkit::result<> unvm::ReloadConfigFile(Config &config)
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

    Config merged;
    if (std::filesystem::exists(path))
    {
        std::ifstream stream(path);
        if (!stream)
        {
            return toolkit::make_error("failed to open config file.");
        }

        json::node json;
        stream >> json;

        if (!(json >> merged))
        {
            return toolkit::make_error("failed to parse config file.");
        }
    }

    MergeConfig(merged, config);

    config.Default = std::move(merged.Default);
    config.Installed = std::move(merged.Installed);
    config.Fingerprints = std::move(merged.Fingerprints);
    config.Tracked = std::move(merged.Tracked);

    return {};
}
