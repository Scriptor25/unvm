#include <http.hxx>
#include <json.hxx>
#include <semver.hxx>
#include <table.hxx>
#include <url.hxx>
#include <util.hxx>

#include <format>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include <archive.h>
#include <archive_entry.h>

#include <version.h>

struct Config
{
    std::filesystem::path InstallDirectory;
    std::filesystem::path ActiveDirectory;

    std::set<std::string> Installed;
    std::optional<std::string> Active;
};

struct OptString
{
    bool HasValue{};
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
    bool Security{};
};

using VersionTable = std::vector<VersionEntry>;

template<>
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
        {
            return false;
        }

        value.InstallDirectory = node.Get("install-directory").To<std::string>();
        value.ActiveDirectory = node.Get("active-directory").To<std::string>();
        value.Installed = node.Get("installed").To<std::set<std::string>>();
        value.Active = node.GetOrNull("active").To<std::optional<std::string>>();

        return true;
    }
};

template<>
struct json::Converter<OptString>
{
    static bool To(const Node &node, OptString &value)
    {
        if (node.IsBoolean())
        {
            if (node.AsBoolean())
            {
                return false;
            }

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

template<>
struct json::Converter<VersionEntry>
{
    static bool To(const Node &node, VersionEntry &value)
    {
        if (!node.IsObject())
        {
            return false;
        }

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
    std::cerr
            << PROJECT_NAME << " - " << PROJECT_TITLE << "\n"
            << "\n"
            << "  Version:    " << PROJECT_VERSION << "\n"
            << "  Build date: " << PROJECT_BUILD_DATE << "\n"
            << "\n"
            << "Usage:\n"
            << "  <version> := latest | lts | v<uint>[.<uint>[.<uint>]] | <lts-name>\n"
            << "\n"
            << "Commands:\n"
            << "  install, i <version>        Install the specified Node.js version\n"
            << "  remove,  r <version>        Remove the specified Node.js version\n"
            << "  use,     u <version>|none   Set active Node.js version, or 'none' to deactivate\n"
            << "  list,    l [available|a]    List installed or available versions\n"
            << "           ls                 Alias for `list`\n"
            << "           la                 Alias for `list available`\n"
            << "\n"
            << "Examples:\n"
            << "  unvm install lts\n"
            << "  unvm install iron\n"
            << "  unvm use v20.3.1\n"
            << "  unvm list available\n"
            << std::endl;
}

constexpr auto MOD_PRESENT_BITS = 0b1000000000000000u;
constexpr auto MOD_VALUE_BITS = 0b0111000000000000u;
constexpr auto OPERATION_BITS = 0b0000000011111111u;

constexpr auto INSTALL_BITS = 0b00000001u;
constexpr auto REMOVE_BITS = 0b00000010u;
constexpr auto USE_BITS = 0b00000100u;
constexpr auto LIST_BITS = 0b00001000u;
constexpr auto WORKSPACE_BITS = 0b00010000u;

constexpr auto LIST_MOD_INSTALLED_BITS = 0b0000000000000000u;
constexpr auto LIST_MOD_AVAILABLE_BITS = 0b0001000000000000u;

/**
 * bits [3:0] -> operation
 * bits [7:4] -> modifier
 *  - bit 7 -> modifier present bit
 *  - bits [6:4] -> modifier value
 */
static const std::map<std::string_view, unsigned> operation_map
{
    { "install", INSTALL_BITS },
    { "i", INSTALL_BITS },
    { "remove", REMOVE_BITS },
    { "r", REMOVE_BITS },
    { "use", USE_BITS },
    { "u", USE_BITS },
    { "list", LIST_BITS },
    { "l", LIST_BITS },
    { "ls", MOD_PRESENT_BITS | LIST_MOD_INSTALLED_BITS | LIST_BITS },
    { "la", MOD_PRESENT_BITS | LIST_MOD_AVAILABLE_BITS | LIST_BITS },
    { "workspace", WORKSPACE_BITS },
    { "w", WORKSPACE_BITS },
};

static int load_version_table(http::HttpClient &client, VersionTable &table, bool online)
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

    auto index = GetDataDirectory() / "index.json";

    if (online || !std::filesystem::exists(index))
    {
        std::stringstream stream;

        http::HttpRequest request = {
            .Method = http::HttpMethod::Get,
            .Location = http::ParseUrl("https://nodejs.org/dist/index.json"),
        };
        http::HttpResponse response = {
            .Body = &stream,
        };

        if (auto error = client.Request(request, response))
        {
            std::cerr << "failed to get file." << std::endl;
            return error;
        }

        if (!http::is_success(response.StatusCode))
        {
            std::cerr
                    << "failed to get file: "
                    << response.StatusCode
                    << ", "
                    << response.StatusMessage
                    << std::endl
                    << stream.str()
                    << std::endl;
            return 1;
        }

        json::Node json;
        stream >> json;

        table = json.To<VersionTable>();

        std::ofstream file(index);
        file << json;

        return 0;
    }

    std::ifstream stream(index);

    json::Node json;
    stream >> json;

    table = json.To<VersionTable>();
    return 0;
}

static std::string to_lower(std::string_view str)
{
    std::string result;
    for (auto &c : str)
    {
        result += static_cast<char>(std::tolower(c));
    }
    return result;
}

static unsigned count_version_segments(std::string_view str)
{
    unsigned segments = 0;

    std::size_t beg = 0, end;
    while ((end = str.find('.', beg)) != std::string_view::npos)
    {
        ++segments;
        beg = end + 1;
    }

    if (beg != str.length())
    {
        ++segments;
    }

    return segments;
}

static const VersionEntry *find_effective_version(const VersionTable &table, std::string_view version)
{
    // latest
    if (version == "latest")
    {
        if (!table.empty())
        {
            return &table.front();
        }
    }
    // latest lts
    else if (version == "lts")
    {
        for (auto &entry : table)
        {
            if (entry.Lts.HasValue)
            {
                return &entry;
            }
        }
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

        switch (count_version_segments(version))
        {
        case 1:
        case 2:
        {
            const auto pattern = std::string(version) + '.';
            for (auto &entry : table)
            {
                if (entry.Version.starts_with(pattern))
                {
                    return &entry;
                }
            }
            break;
        }

        case 3:
            for (auto &entry : table)
            {
                if (entry.Version == version)
                {
                    return &entry;
                }
            }
            break;

        default:
            break;
        }
    }
    // lts by name
    else
    {
        const auto name = to_lower(version);
        for (auto &entry : table)
        {
            if (entry.Lts.HasValue && to_lower(entry.Lts.Value) == name)
            {
                return &entry;
            }
        }
    }

    return nullptr;
}

static la_ssize_t read_callback(archive */*arc*/, void *user_data, const void **buffer)
{
    static char buf[0x4000];

    const auto stream = static_cast<std::istream *>(user_data);

    stream->read(buf, sizeof(buf));
    const auto len = stream->gcount();

    if (len <= 0)
    {
        *buffer = nullptr;
        return 0;
    }

    *buffer = buf;
    return len;
}

static int unpack(std::istream &stream, const std::filesystem::path &directory)
{
    const auto arc = archive_read_new();
    const auto ext = archive_write_disk_new();

    archive_read_support_format_all(arc);
    archive_read_support_filter_all(arc);

    archive_write_disk_set_options(
        ext,
        ARCHIVE_EXTRACT_TIME
        | ARCHIVE_EXTRACT_PERM
        | ARCHIVE_EXTRACT_ACL
        | ARCHIVE_EXTRACT_FFLAGS);

    if (const auto error = archive_read_open(arc, &stream, nullptr, read_callback, nullptr))
    {
        std::cerr << "failed to open archive: " << archive_error_string(arc) << std::endl;

        archive_read_free(arc);
        archive_write_free(ext);
        return error;
    }

    archive_entry *entry;
    const void *buf;
    std::size_t len;
    la_int64_t off;

    int err;
    while (!((err = archive_read_next_header(arc, &entry))))
    {
        auto pathname = directory / archive_entry_pathname(entry);
        auto pathname_string = pathname.string();
        archive_entry_set_pathname(entry, pathname_string.c_str());

        if (const auto error = archive_write_header(ext, entry))
        {
            std::cerr << "failed to write archive header: " << archive_error_string(ext) << std::endl;

            archive_read_free(arc);
            archive_write_free(ext);
            return error;
        }

        while (true)
        {
            if (const auto error = archive_read_data_block(arc, &buf, &len, &off))
            {
                if (error == ARCHIVE_EOF)
                {
                    break;
                }

                std::cerr << "failed to read archive data block: " << archive_error_string(arc) << std::endl;

                archive_read_free(arc);
                archive_write_free(ext);
                return error;
            }

            if (const auto error = archive_write_data_block(ext, buf, len, off))
            {
                std::cerr << "failed to write archive data block: " << archive_error_string(ext) << std::endl;

                archive_read_free(arc);
                archive_write_free(ext);
                return static_cast<int>(error);
            }
        }
    }

    if (err != ARCHIVE_EOF)
    {
        std::cerr << "failed to read archive header: " << archive_error_string(arc) << std::endl;

        archive_read_free(arc);
        archive_write_free(ext);
        return err;
    }

    archive_read_free(arc);
    archive_write_free(ext);
    return 0;
}

static int install(Config &config, http::HttpClient &client, std::string_view version, const VersionEntry &entry)
{
    if (config.Installed.contains(entry.Version))
    {
        std::cerr << "version '" << version << "' is already installed." << std::endl;
        return 0;
    }

#if defined(SYSTEM_WINDOWS)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-win-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-win-arm64";
#endif

    constexpr auto ending = "zip";

#endif

#if defined(SYSTEM_LINUX)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-linux-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-linux-arm64";
#endif

    constexpr auto ending = "tar.xz";

#endif

#if defined(SYSTEM_DARWIN)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-darwin-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-darwin-arm64";
#endif

    constexpr auto ending = "tar.xz";

#endif

    auto filename = std::format(format, entry.Version);
    auto pathname = std::format("/dist/{}/{}.{}", entry.Version, filename, ending);

    std::stringstream stream(std::stringstream::binary | std::stringstream::in | std::stringstream::out);

    http::HttpRequest request = {
        .Method = http::HttpMethod::Get,
        .Location = {
            .UseTLS = true,
            .Host = "nodejs.org",
            .Port = 443,
            .Pathname = pathname,
        },
    };
    http::HttpResponse response = {
        .Body = &stream,
    };

    if (auto error = client.Request(request, response))
    {
        std::cerr << "failed to get file." << std::endl;
        return error;
    }

    if (!http::is_success(response.StatusCode))
    {
        std::cerr
                << "failed to get file: "
                << response.StatusCode
                << ", "
                << response.StatusMessage
                << std::endl
                << stream.str()
                << std::endl;
        return 1;
    }

    std::filesystem::create_directories(config.InstallDirectory);

    if (const auto error = unpack(stream, config.InstallDirectory))
    {
        std::cerr << "failed to unpack archive." << std::endl;
        return error;
    }

    std::filesystem::rename(config.InstallDirectory / filename, config.InstallDirectory / entry.Version);

    config.Installed.emplace(entry.Version);

    return 0;
}

static int install(Config &config, http::HttpClient &client, std::string_view version)
{
    VersionTable table;
    if (auto error = load_version_table(client, table, true))
    {
        std::cerr << "failed to load version table." << std::endl;
        return error;
    }

    auto entry_ptr = find_effective_version(table, version);
    if (!entry_ptr)
    {
        std::cerr << "no effective version for '" << version << "'." << std::endl;
        return 1;
    }

    return install(config, client, version, *entry_ptr);
}

static int remove(Config &config, http::HttpClient &client, std::string_view version)
{
    VersionTable table;
    if (const auto error = load_version_table(client, table, false))
    {
        return error;
    }

    const auto entry_ptr = find_effective_version(table, version);

    if (!entry_ptr || !config.Installed.contains(entry_ptr->Version))
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 0;
    }

    auto &entry = *entry_ptr;
    if (config.Active.has_value() && config.Active.value() == entry.Version)
    {
        std::cerr << "version '" << version << "' is still in use." << std::endl;
        return 1;
    }

    std::filesystem::remove_all(config.InstallDirectory / entry.Version);

    config.Installed.erase(entry.Version);
    return 0;
}

static int use(Config &config, http::HttpClient &client, const std::string_view &version, const VersionEntry &entry)
{
    if (config.Active.has_value())
    {
        if (config.Active.value() == entry.Version)
        {
            std::cerr << "version '" << version << "' is already installed and active." << std::endl;
            return 0;
        }

        if (const auto error = RemoveLink(config.ActiveDirectory))
        {
            std::cerr << "failed to un-link current active version '" << config.Active.value() << "'." << std::endl;
            return error;
        }
    }

    if (const auto error = CreateLink(config.ActiveDirectory, config.InstallDirectory / entry.Version))
    {
        std::cerr << "failed to link new active version '" << version << "'." << std::endl;
        return error;
    }

    if (const auto error = AppendUserPath(config.ActiveDirectory))
    {
        std::cerr << "failed to append active directory to user path variable." << std::endl;
        return error;
    }

    config.Active = entry.Version;
    return 0;
}

static int use(Config &config, http::HttpClient &client, std::string_view version)
{
    if (version == "none")
    {
        if (!config.Active.has_value())
        {
            std::cerr << "node is already inactive." << std::endl;
            return 0;
        }

        if (const auto error = RemoveLink(config.ActiveDirectory))
        {
            std::cerr << "failed to un-link current active version '" << config.Active.value() << "'." << std::endl;
            return error;
        }

        config.Active = std::nullopt;
        return 0;
    }

    VersionTable table;
    if (const auto error = load_version_table(client, table, false))
    {
        return error;
    }

    const auto entry_ptr = find_effective_version(table, version);

    if (!entry_ptr || !config.Installed.contains(entry_ptr->Version))
    {
        std::cerr << "version '" << version << "' is not installed." << std::endl;
        return 1;
    }
    
    return use(config, client, version, *entry_ptr);
}

static int list(Config &config, http::HttpClient &client, const bool available)
{
    Table out(
        {
            { "", true },
            { "Lts", true },
            { "Version", true },
            { "Npm", true },
            { "Date", true },
            { "Modules", false }
        });

    VersionTable table;
    if (const auto error = load_version_table(client, table, available))
    {
        return error;
    }

    for (auto &entry : table)
    {
        if (available || config.Installed.contains(entry.Version))
        {
            out
                    << (config.Active.has_value() && config.Active.value() == entry.Version ? "*" : "")
                    << (entry.Lts.HasValue ? entry.Lts.Value : "")
                    << entry.Version
                    << (entry.Npm.has_value() ? entry.Npm.value() : "")
                    << entry.Date
                    << (entry.Modules.has_value() ? entry.Modules.value() : "");
        }
    }

    if (out.Empty())
    {
        std::cerr << "no elements to list." << std::endl;
        return 0;
    }

    std::cerr << out;
    return 0;
}

static int workspace(Config &config, http::HttpClient &client)
{
    auto package_json = std::filesystem::weakly_canonical("./package.json");

    if (!std::filesystem::exists(package_json))
    {
        std::cerr << "no package.json in current directory." << std::endl;
        return 1;
    }

    std::ifstream stream(package_json);
    
    if (!stream)
    {
        std::cerr << "failed to open package.json." << std::endl;
        return 1;
    }

    auto node = json::Parser::Parse(stream);
    auto maybe_version = node.GetOrNull("engines").GetOrNull("node").To<std::optional<std::string>>();

    auto version = maybe_version.has_value() ? maybe_version.value() : "*";
    auto set = semver::ParseRangeSet(version);

    VersionTable table;
    if (const auto error = load_version_table(client, table, false))
    {
        return error;
    }

    for (auto &entry : table)
    {
        if (!config.Installed.contains(entry.Version))
        {
            continue;
        }

        if (!semver::IsInRange(set, entry.Version))
        {
            continue;
        }

        return use(config, client, version, entry);
    }

    if (const auto error = load_version_table(client, table, true))
    {
        return error;
    }
    
    for (auto &entry : table)
    {
        if (!semver::IsInRange(set, entry.Version))
        {
            continue;
        }

        if (const auto error = install(config, client, version, entry))
        {
            return error;
        }

        if (const auto error = use(config, client, version, entry))
        {
            return error;
        }

        return 0;
    }

    std::cerr << "no version match for '" << version << "'." << std::endl;
    return 1;
}

static int execute(const std::vector<std::string_view> &args)
{
    if (args.empty())
    {
        print();
        return 0;
    }

    if (!operation_map.contains(args[0]))
    {
        std::cerr << "undefined operation '" << args[0] << "'." << std::endl;
        return 1;
    }

    auto operation = operation_map.at(args[0]);

    Config config;
    if (std::ifstream stream(GetDataDirectory() / "config.json"); stream)
    {
        json::Node json;
        stream >> json;

        config = json.To<Config>();
    }
    else
    {
        config.InstallDirectory = GetDataDirectory() / "version";
        config.ActiveDirectory = GetDataDirectory() / "active";
    }

    http::HttpClient client;

    int code;
    switch (operation & OPERATION_BITS)
    {
    case INSTALL_BITS:
        if (args.size() != 2)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        code = install(config, client, args[1]);
        break;

    case REMOVE_BITS:
        if (args.size() != 2)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        code = remove(config, client, args[1]);
        break;

    case USE_BITS:
        if (args.size() != 2)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        code = use(config, client, args[1]);
        break;

    case LIST_BITS:
    {
        bool available;
        if (operation & MOD_PRESENT_BITS)
        {
            if (args.size() != 1)
            {
                std::cerr << "invalid argument count." << std::endl;
                return 1;
            }
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
                if (args[1] != "available")
                {
                    std::cerr << "invalid list modifier '" << args[1] << "'." << std::endl;
                    return 1;
                }
                available = true;
                break;

            default:
                std::cerr << "invalid argument count." << std::endl;
                return 1;
            }
        }

        code = list(config, client, available);
        break;
    }

    case WORKSPACE_BITS:
        if (args.size() != 1)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        code = workspace(config, client);
        break;

    default:
        code = 1;
        break;
    }

    {
        auto parent = GetDataDirectory();
        std::filesystem::create_directories(parent);

        std::ofstream stream(parent / "config.json");
        if (!stream)
        {
            std::cerr << "failed to open config." << std::endl;
            return 1;
        }

        auto json = json::Node::From(config);
        stream << json;
    }

    return code;
}

int main(const int argc, const char *const *argv)
{
    return execute({ argv + 1, argv + argc });
}
