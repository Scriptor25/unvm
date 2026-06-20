#include <unvm/json.hxx>
#include <unvm/util.hxx>

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

    ok &= node["default"] >> value.Default;
    ok &= from_data_opt(node["installed"], value.Installed);
    ok &= from_data_opt(node["fingerprints"], value.Fingerprints);

    return ok;
}

void data::serializer<unvm::Config>::to_data(json::Node &node, const unvm::Config &value)
{
    node = json::Node::Map
    {
        { "installed", value.Installed },
        { "default", value.Default },
        { "fingerprints", value.Fingerprints },
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
    ok &= node["npm"] >> value.NPM;
    ok &= node["v8"] >> value.V8;
    ok &= node["uv"] >> value.UV;
    ok &= node["zlib"] >> value.ZLib;
    ok &= node["openssl"] >> value.OpenSSL;

    std::optional<std::string> modules;
    ok &= node["modules"] >> modules;

    if (modules)
    {
        auto &m = *modules;

        int base;

        if (m.starts_with("0b"))
        {
            base = 2;
            m = m.substr(2);
        }
        else if (m.starts_with("0x"))
        {
            base = 16;
            m = m.substr(2);
        }
        else
        {
            base = 10;
        }

        if (const auto res = unvm::ParseString<uint16_t>(m, base) >> value.Modules; !res)
        {
            ok = false;
        }
    }

    auto &lts = node["lts"];
    if (bool val; lts >> val && !val)
    {
        value.LTS = std::nullopt;
    }
    else
    {
        ok &= lts >> value.LTS;
    }

    ok &= node["security"] >> value.Security;

    return ok;
}
