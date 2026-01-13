#include <format>
#include <iostream>
#include <set>
#include <http.hxx>
#include <json.hxx>
#include <table.hxx>
#include <bit7z/bitarchivereader.hpp>
#include <fstream>
#include <assert.hxx>
#include <path.hxx>
#include <link.hxx>
#include <env.hxx>

struct Config
{
    std::filesystem::path InstallDirectory;
    std::filesystem::path ActiveDirectory;

    std::set<std::string> Installed;
    std::optional<std::string> Active;
};

struct OptString
{
    bool HasValue;
    std::string Value;
};

struct VersionEntry
{
    std::string Version;
    std::string Date;
    std::vector<std::string> Files;
    std::optional<std::string> Npm;
    std::string V8;
    std::optional<std::string> Uv;
    std::optional<std::string> ZLib;
    std::optional<std::string> OpenSSL;
    std::optional<std::string> Modules;
    OptString Lts;
    bool Security;
};

using VersionTable = std::vector<VersionEntry>;

template <>
struct json::Converter<Config>
{
    static bool From(Node &node, const Config &value)
    {
        std::map<std::string, Node> entries;
        entries["install-directory"] = Node::From(value.InstallDirectory.string());
        entries["active-directory"] = Node::From(value.ActiveDirectory.string());
        entries["installed"] = Node::From(value.Installed);
        entries["active"] = Node::From(value.Active);

        node.Type = NodeType::Object;
        node.Value = entries;
        return true;
    }

    static bool To(const Node &node, Config &value)
    {
        if (!node.IsObject())
            return false;

        value.InstallDirectory = node.Get("install-directory").To<std::string>();
        value.ActiveDirectory = node.Get("active-directory").To<std::string>();
        value.Installed = node.Get("installed").To<std::set<std::string>>();
        value.Active = node.GetOrNull("active").To<std::optional<std::string>>();

        return true;
    }
};

template <>
struct json::Converter<OptString>
{
    static bool To(const Node &node, OptString &value)
    {
        if (node.IsBoolean())
        {
            auto &b = node.AsBoolean();
            if (b)
                return false;

            value.HasValue = false;
            return true;
        }

        if (node.IsString())
        {
            value.HasValue = true;
            value.Value = node.AsString();
            return true;
        }

        return false;
    }
};

template <>
struct json::Converter<VersionEntry>
{
    static bool To(const Node &node, VersionEntry &value)
    {
        if (!node.IsObject())
            return false;

        value.Version = node.Get("version").To<std::string>();
        value.Date = node.Get("date").To<std::string>();
        value.Files = node.Get("files").To<std::vector<std::string>>();
        value.Npm = node.GetOrNull("npm").To<std::optional<std::string>>();
        value.V8 = node.GetOrNull("v8").To<std::string>();
        value.Uv = node.GetOrNull("uv").To<std::optional<std::string>>();
        value.ZLib = node.GetOrNull("zlib").To<std::optional<std::string>>();
        value.OpenSSL = node.GetOrNull("openssl").To<std::optional<std::string>>();
        value.Modules = node.GetOrNull("modules").To<std::optional<std::string>>();
        value.Lts = node.Get("lts").To<OptString>();
        value.Security = node.Get("security").To<bool>();

        return true;
    }
};

static void print()
{
    std::cout << "unvm"
              << std::endl
              << std::endl
              << "<version>: latest, lts, v<uint>[.<uint>[.<uint>]], <lts-name>"
              << std::endl
              << std::endl
              << "\t- install, i <version>  - download and install node package"
              << std::endl
              << "\t- remove, r <version>   - remove node package"
              << std::endl
              << "\t- use, u <version>|none - set node package as active, or inactive by using 'none'"
              << std::endl
              << "\t- list, l [available]   - list installed or available packages"
              << std::endl
              << "\t- ls                    - short for `list` or `l`"
              << std::endl
              << "\t- la                    - short for `list available` or `l available`"
              << std::endl;
}

constexpr unsigned MOD_PRESENT_BITS = 0b10000000u;
constexpr unsigned MOD_VALUE_BITS = 0b01110000u;
constexpr unsigned OPERATION_BITS = 0b00001111u;

constexpr unsigned INSTALL_BITS = 0b0001u;
constexpr unsigned REMOVE_BITS = 0b0010u;
constexpr unsigned USE_BITS = 0b0100u;
constexpr unsigned LIST_BITS = 0b1000u;

constexpr unsigned LIST_MOD_INSTALLED_BITS = 0b00000000u;
constexpr unsigned LIST_MOD_AVAILABLE_BITS = 0b00010000u;

/**
 * bits [3:0] -> operation
 * bits [7:4] -> modifier
 *  - bit 7 -> modifier present bit
 *  - bits [6:4] -> modifier value
 */
static const std::map<std::string_view, unsigned> operation_map = {
    {"install", INSTALL_BITS},
    {"i", INSTALL_BITS},
    {"remove", REMOVE_BITS},
    {"r", REMOVE_BITS},
    {"use", USE_BITS},
    {"u", USE_BITS},
    {"list", LIST_BITS},
    {"l", LIST_BITS},
    {"ls", MOD_PRESENT_BITS | LIST_MOD_INSTALLED_BITS | LIST_BITS},
    {"la", MOD_PRESENT_BITS | LIST_MOD_AVAILABLE_BITS | LIST_BITS},
};

static std::string to_lower(const std::string_view &str)
{
    std::string result;
    for (auto &c : str)
        result += std::tolower(c);
    return result;
}

static unsigned count_version_segments(const std::string_view &str)
{
    unsigned segments = 0;

    std::size_t beg = 0, end;
    while ((end = str.find('.', beg)) != std::string_view::npos)
    {
        ++segments;
        beg = end + 1;
    }

    if (beg != str.length())
        ++segments;

    return segments;
}

static const VersionEntry &find_effective_version(const VersionTable &table, const std::string_view &version)
{
    // latest
    if (version == "latest")
    {
        if (!table.empty())
            return table.front();
    }
    // latest lts
    else if (version == "lts")
    {
        for (auto &entry : table)
            if (entry.Lts.HasValue)
                return entry;
    }
    // version by pattern
    else if (version.starts_with('v'))
    {
        // vX     -> vX.L.L
        // vX.X   -> vX.X.L
        // vX.X.X -> vX.X.X
        // vX.X.X -> vX.X.X
        // (X = some version)
        // (L = latest version)

        auto segments = count_version_segments(version);

        switch (segments)
        {
        case 1:
        case 2:
        {
            auto pattern = std::string(version) + '.';
            for (auto &entry : table)
                if (entry.Version.starts_with(pattern))
                    return entry;
            break;
        }

        case 3:
            for (auto &entry : table)
                if (entry.Version == version)
                    return entry;
            break;

        default:
            break;
        }
    }
    // lts by name
    else
    {
        auto name = to_lower(version);
        for (auto &entry : table)
            if (entry.Lts.HasValue && to_lower(entry.Lts.Value) == name)
                return entry;
    }

    Error("no effective version for '{}'.", version);
}

static int install(Config &config, http::Client &client, const VersionTable &table, const std::string_view &version)
{
    auto &entry = find_effective_version(table, version);

    if (config.Installed.contains(entry.Version))
    {
        std::cout << "version '" << version << "' is already installed." << std::endl;
        return 0;
    }

    auto full_name = std::format("node-{}-win-x64", entry.Version);
    auto url = std::format("http://nodejs.org/dist/{}/{}.7z", entry.Version, full_name);

    std::stringstream stream(std::stringstream::binary | std::stringstream::in | std::stringstream::out);

    auto status = client.Request(http::Method::Get, url, {}, nullptr, &stream);
    Assert(200 <= status && status <= 299, "failed to get file.");

    bit7z::Bit7zLibrary library("C:\\Program Files\\7-Zip\\7z.dll");
    bit7z::BitArchiveReader archive(library, stream, bit7z::BitFormat::SevenZip);

    std::filesystem::path parent(config.InstallDirectory);
    std::filesystem::create_directories(parent);

    archive.extractTo(parent.string());

    std::filesystem::rename(parent / full_name, parent / entry.Version);

    config.Installed.emplace(entry.Version);

    return 0;
}

static int remove(Config &config, http::Client &client, const VersionTable &table, const std::string_view &version)
{
    auto &entry = find_effective_version(table, version);

    if (!config.Installed.contains(entry.Version))
    {
        std::cout << "version '" << version << "' is not installed." << std::endl;
        return 0;
    }

    if (config.Active.has_value() && config.Active.value() == entry.Version)
    {
        std::cout << "version '" << version << "' is still in use." << std::endl;
        return 1;
    }

    std::filesystem::path parent(config.InstallDirectory);
    std::filesystem::remove_all(parent / entry.Version);

    config.Installed.erase(entry.Version);
    return 0;
}

static int use(Config &config, http::Client &client, const VersionTable &table, const std::string_view &version)
{
    if (version == "none")
    {
        if (!config.Active.has_value())
        {
            std::cout << "node is already inactive." << std::endl;
            return 0;
        }

        auto remove_link_ok = RemoveLink(config.ActiveDirectory);
        Assert(remove_link_ok, "failed to un-link current active version '{}'.", config.Active.value());

        config.Active = std::nullopt;
        return 0;
    }

    auto &entry = find_effective_version(table, version);

    if (!config.Installed.contains(entry.Version))
    {
        std::cout << "version '" << version << "' is not installed." << std::endl;
        return 1;
    }

    if (config.Active.has_value())
    {
        if (config.Active.value() == entry.Version)
        {
            std::cout << "version '" << version << "' is already installed and active." << std::endl;
            return 0;
        }

        auto remove_link_ok = RemoveLink(config.ActiveDirectory);
        Assert(remove_link_ok, "failed to un-link current active version '{}'.", config.Active.value());
    }

    auto create_link_ok = CreateLink(config.ActiveDirectory, config.InstallDirectory / entry.Version);
    Assert(create_link_ok, "failed to link new active version '{}'.", version);

    auto append_path_ok = AppendUserPath(config.ActiveDirectory);
    Assert(append_path_ok, "failed to append active directory to user path variable");

    config.Active = entry.Version;
    return 0;
}

static int list(Config &config, http::Client &client, const VersionTable &table, bool available)
{
    Table out({{"Lts", true}, {"Version", true}, {"Npm", true}, {"Date", true}, {"Modules", false}});

    for (auto &entry : table)
    {
        if (available || config.Installed.contains(entry.Version))
        {
            out << (entry.Lts.HasValue ? entry.Lts.Value : "")
                << entry.Version
                << (entry.Npm.has_value() ? entry.Npm.value() : "")
                << entry.Date
                << (entry.Modules.has_value() ? entry.Modules.value() : "");
        }
    }

    if (out.Empty())
    {
        std::cout << "no elements to list." << std::endl;
        return 0;
    }

    std::cout << out;
    return 0;
}

static int execute(const std::vector<std::string_view> &args)
{
    if (args.empty())
    {
        print();
        return 0;
    }

    Assert(operation_map.contains(args[0]), "undefined operation '{}'.", args[0]);

    auto operation = operation_map.at(args[0]);

    Config config;
    if (std::ifstream stream(get_data_directory() / "config.json"); stream)
    {
        json::Node json;
        stream >> json;

        config = json.To<Config>();
    }
    else
    {
        config.InstallDirectory = get_data_directory() / "version";
        config.ActiveDirectory = get_data_directory() / "active";
    }

    http::Client client;

    VersionTable table;
    {
        /**
         * {
         *   version:  string
         *   date:     string
         *   files:    string[]
         *   npm?:     string
         *   v8:       string
         *   uv?:      string
         *   zlib?:    string
         *   openssl?: string
         *   modules?: string
         *   lts:      string | false
         *   security: boolean
         * }[]
         */

        std::stringstream stream;
        auto status = client.Request(http::Method::Get, "http://nodejs.org/dist/index.json", {}, nullptr, &stream);
        Assert(200 <= status && status <= 299, "failed to get index.");

        json::Node json;
        stream >> json;

        table = json.To<VersionTable>();
    }

    int code;
    switch (operation & OPERATION_BITS)
    {
    case INSTALL_BITS:
        Assert(args.size() == 2, "invalid argument count");
        code = install(config, client, table, args[1]);
        break;

    case REMOVE_BITS:
        Assert(args.size() == 2, "invalid argument count");
        code = remove(config, client, table, args[1]);
        break;

    case USE_BITS:
        Assert(args.size() == 2, "invalid argument count");
        code = use(config, client, table, args[1]);
        break;

    case LIST_BITS:
    {
        bool available;
        if (operation & MOD_PRESENT_BITS)
        {
            Assert(args.size() == 1, "invalid argument count");
            available = (operation & MOD_VALUE_BITS) == LIST_MOD_AVAILABLE_BITS;
        }
        else
        {
            switch (args.size())
            {
            case 1:
                available = false;
                break;

            case 2:
                Assert(std::string_view(args[1]) == "available", "invalid list modifier '{}'", args[1]);
                available = true;
                break;

            default:
                Error("invalid argument count.");
            }
        }

        code = list(config, client, table, available);
        break;
    }

    default:
        code = 1;
        break;
    }

    {
        auto parent = get_data_directory();
        std::filesystem::create_directories(parent);

        std::ofstream stream(parent / "config.json");
        Assert(!!stream, "failed to open config.");

        auto json = json::Node::From(config);
        stream << json;
    }

    return code;
}

int main(const int argc, const char *const *argv)
{
    return execute({argv + 1, argv + argc});
}
