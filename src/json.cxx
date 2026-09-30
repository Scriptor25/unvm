#include <unvm/json.hxx>
#include <unvm/util.hxx>

bool data::serializer<json::node, std::filesystem::path>::from_data(
    const json::node &node,
    std::filesystem::path &value)
{
    if (json::string s; node >> s)
    {
        value = std::move(s);
        return true;
    }

    return false;
}

void data::serializer<json::node, std::filesystem::path>::to_data(
    json::node &node,
    const std::filesystem::path &value)
{
    node = value.string();
}

bool data::serializer<json::node, unvm::Config>::from_data(
    const json::node &node,
    unvm::Config &value)
{
    if (!node.is<json::object>())
    {
        return false;
    }

    auto ok = true;

    ok &= node["default"] >> value.Default;

    if (auto &installed = node["installed"]; installed.is<json::array>())
    {
        if (std::unordered_set<std::string> set; installed >> set)
        {
            for (auto &entry : set)
            {
                value.Installed[entry] = false;
            }
        }
        else
        {
            ok = false;
        }
    }
    else
    {
        ok &= from_data_opt(node["installed"], value.Installed);
    }

    ok &= from_data_opt(node["fingerprints"], value.Fingerprints);
    ok &= from_data_opt(node["tracked"], value.Tracked);

    return ok;
}

void data::serializer<json::node, unvm::Config>::to_data(
    json::node &node,
    const unvm::Config &value)
{
    node = json::object
    {
        { "default", value.Default },
        { "installed", value.Installed },
        { "fingerprints", value.Fingerprints },
        { "tracked", value.Tracked },
    };
}

bool data::serializer<json::node, unvm::VersionEntry>::from_data(
    const json::node &node,
    unvm::VersionEntry &value)
{
    if (!node.is<json::object>())
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
