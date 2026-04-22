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
    {
        return false;
    }

    auto ok = true;

    ok &= node["installed"] >> value.Installed;
    ok &= node["default"] >> value.Default;

    return ok;
}

void data::serializer<unvm::Config>::to_data(json::Node &node, const unvm::Config &value)
{
    node = json::Node::Map
    {
        { "installed", value.Installed },
        { "default", value.Default },
    };
}

bool data::serializer<unvm::VersionEntry>::from_data(const json::Node &node, unvm::VersionEntry &value)
{
    if (!node.Is<json::Node::Map>())
    {
        return false;
    }

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
    ok &= node["security"] >> value.Security;

    auto &lts = node["lts"];
    if (bool val; lts >> val && !val)
    {
        value.Lts = std::nullopt;
    }
    else
    {
        ok &= lts >> value.Lts;
    }

    return ok;
}
