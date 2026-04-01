#include <unvm/json.hxx>

bool json::serializer<std::filesystem::path>::from_json(const Node &node, std::filesystem::path &value)
{
    if (String s; node >> s)
    {
        value = std::move(s);
        return true;
    }

    return false;
}

void json::serializer<std::filesystem::path>::to_json(Node &node, const std::filesystem::path &value)
{
    node = value.string();
}

bool json::serializer<unvm::Config>::from_json(const Node &node, unvm::Config &value)
{
    if (!node.Is<Object>())
        return false;

    auto ok = true;

    ok &= node["install-directory"] >> value.InstallDirectory;
    ok &= node["active-directory"] >> value.ActiveDirectory;
    ok &= node["installed"] >> value.Installed;
    ok &= node["active"] >> value.Active;

    return ok;
}

void json::serializer<unvm::Config>::to_json(Node &node, const unvm::Config &value)
{
    node = Object
    {
        { "install-directory", value.InstallDirectory },
        { "active-directory", value.ActiveDirectory },
        { "installed", value.Installed },
        { "active", value.Active },
    };
}

bool json::serializer<unvm::StringOrFalse>::from_json(const Node &node, unvm::StringOrFalse &value)
{
    if (Boolean val; node >> val)
    {
        if (val)
            return false;

        value.HasValue = false;
        return true;
    }

    if (String val; node >> val)
    {
        value.HasValue = true;
        value.Value = std::move(val);
        return true;
    }

    return false;
}

bool json::serializer<unvm::VersionEntry>::from_json(const Node &node, unvm::VersionEntry &value)
{
    if (!node.Is<Object>())
        return false;

    auto ok = true;

    ok &= node["version"] >> value.Version;
    ok &= node["date"] >> value.Date;
    ok &= node["files"] >> value.Files;
    ok &= node["npm"] >> value.Npm;
    ok &= node["v8"] >> value.V8;
    ok &= node["uv"] >> value.Uv;
    ok &= node["zlib"] >> value.ZLib;
    ok &= node["openssl"] >> value.OpenSSL;
    ok &= node["modules"] >> value.Modules;
    ok &= node["lts"] >> value.Lts;
    ok &= node["security"] >> value.Security;

    return ok;
}
