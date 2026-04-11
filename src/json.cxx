#include <unvm/json.hxx>

bool data::serializer<std::filesystem::path>::from_data(const json::Node &node, std::filesystem::path &value)
{
    if (json::String s; node >> s)
    {
        value = std::move(s);
        return true;
    }

    return false;
}

void data::serializer<std::filesystem::path>::to_data(json::Node &node, const std::filesystem::path &value)
{
    node = value.string();
}

bool data::serializer<unvm::Config>::from_data(const json::Node &node, unvm::Config &value)
{
    if (!node.Is<json::Node::Map>())
        return false;

    auto ok = true;

    ok &= node["install-directory"] >> value.InstallDirectory;
    ok &= node["active-directory"] >> value.ActiveDirectory;
    ok &= node["installed"] >> value.Installed;
    ok &= node["active"] >> value.Active;

    return ok;
}

void data::serializer<unvm::Config>::to_data(json::Node &node, const unvm::Config &value)
{
    node = json::Node::Map
    {
        { "install-directory", value.InstallDirectory },
        { "active-directory", value.ActiveDirectory },
        { "installed", value.Installed },
        { "active", value.Active },
    };
}

bool data::serializer<unvm::StringOrFalse>::from_data(const json::Node &node, unvm::StringOrFalse &value)
{
    if (json::Boolean val; node >> val)
    {
        if (val)
            return false;

        value.HasValue = false;
        return true;
    }

    if (json::String val; node >> val)
    {
        value.HasValue = true;
        value.Value = std::move(val);
        return true;
    }

    return false;
}

bool data::serializer<unvm::VersionEntry>::from_data(const json::Node &node, unvm::VersionEntry &value)
{
    if (!node.Is<json::Node::Map>())
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
