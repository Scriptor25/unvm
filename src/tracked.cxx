#include <unvm/unvm.hxx>

toolkit::result<> unvm::Track(
    Config &config,
    http::HttpClient &client,
    const std::string_view name,
    const std::string_view version)
{
    if (const auto it = config.Tracked.find(std::string(name)); it != config.Tracked.end())
    {
        return toolkit::make_error("tracked version with name '{}' already exists.", name);
    }

    if (auto res = Install(config, client, version); !res)
    {
        return res;
    }

    config.Tracked.insert({ std::string(name), std::string(version) });
    config.AddedTracked.insert(std::string(name));
    return {};
}

toolkit::result<> unvm::Untrack(Config &config, http::HttpClient &client, const std::string_view name, const bool prune)
{
    const auto it = config.Tracked.find(std::string(name));
    if (it == config.Tracked.end())
    {
        return toolkit::make_error("no tracked version with name '{}'.", name);
    }

    const auto &version = it->second;

    if (prune)
    {
        if (auto res = Remove(config, client, version); !res)
        {
            return res;
        }
    }

    config.Tracked.erase(it);
    config.RemovedTracked.insert(std::string(name));
    return {};
}
