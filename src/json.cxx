#include <unvm/json.hxx>

template<>
bool from_json(const json::Node &node, std::filesystem::path &value)
{
    if (std::string s; from_json(node, s))
    {
        value = std::move(s);
        return true;
    }
    return false;
}

template<>
void to_json(json::Node &node, const std::filesystem::path &value)
{
    to_json(node, value.string());
}

template<>
bool from_json(const json::Node &node, unvm::Config &value)
{
    if (!node.IsObject())
        return false;

    auto ok = true;

    ok &= from_json(node["install-directory"], value.InstallDirectory);
    ok &= from_json(node["active-directory"], value.ActiveDirectory);
    ok &= from_json(node["installed"], value.Installed);
    ok &= from_json(node["active"], value.Active);

    return ok;
}

template<>
void to_json(json::Node &node, const unvm::Config &value)
{
    std::map<std::string, json::Node> entries;

    to_json(entries["install-directory"], value.InstallDirectory);
    to_json(entries["active-directory"], value.ActiveDirectory);
    to_json(entries["installed"], value.Installed);
    to_json(entries["active"], value.Active);

    node = { std::move(entries) };
}

template<>
bool from_json(const json::Node &node, unvm::StringOrFalse &value)
{
    if (bool val; from_json(node, val))
    {
        if (val)
            return false;

        value.HasValue = false;
        return true;
    }

    if (std::string val; from_json(node, val))
    {
        value.HasValue = true;
        value.Value = std::move(val);
        return true;
    }

    return false;
}

template<>
bool from_json(const json::Node &node, unvm::VersionEntry &value)
{
    if (!node.IsObject())
        return false;

    from_json(node["version"], value.Version);
    from_json(node["date"], value.Date);
    from_json(node["files"], value.Files);
    from_json(node["npm"], value.Npm);
    from_json(node["v8"], value.V8);
    from_json(node["uv"], value.Uv);
    from_json(node["zlib"], value.ZLib);
    from_json(node["openssl"], value.OpenSSL);
    from_json(node["modules"], value.Modules);
    from_json(node["lts"], value.Lts);
    from_json(node["security"], value.Security);

    return true;
}
