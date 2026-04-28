#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <openssl/sha.h>

#include <iostream>
#include <sstream>

static toolkit::result<> get_checksum(
    unvm::http::HttpClient &client,
    const unvm::VersionEntry &entry,
    const std::string &filename,
    const std::string &extension,
    std::string &checksum)
{
    auto with_extension = filename + '.' + extension;

    auto pathname = std::format("/dist/{}/SHASUMS256.txt", entry.Version);

    std::stringstream stream(std::stringstream::in | std::stringstream::out);

    unvm::http::HttpRequest request
    {
        .Method = unvm::http::HttpMethod::Get,
        .Location = {
            .UseTLS = true,
            .Host = "nodejs.org",
            .Port = 443,
            .Pathname = pathname,
        },
    };

    unvm::http::HttpResponse response
    {
        .Body = &stream,
    };

    if (auto res = client.Fetch(std::move(request), response); !res)
    {
        return toolkit::make_error("failed to get file: {}", res.error());
    }

    if (!unvm::http::IsSuccess(response.StatusCode))
    {
        return toolkit::make_error("failed to get file: {}, {}\n{}", response.StatusCode, response.StatusMessage, stream.str());
    }

    for (std::string line; std::getline(stream, line);)
    {
        line = unvm::Trim(std::move(line));
        if (line.empty())
        {
            continue;
        }

        if (auto pos = line.find(with_extension); pos != std::string::npos)
        {
            checksum = unvm::Trim(line.substr(0, pos));
            return {};
        }
    }

    return toolkit::make_error("failed to get checksum for filename '{}'.", with_extension);
}

static void generate_checksum(std::istream &str, std::string &checksum)
{
    str.seekg(0, std::ios::end);
    std::vector<unsigned char> buffer(str.tellg());
    str.seekg(0, std::ios::beg);
    str.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
    str.seekg(0, std::ios::beg);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(buffer.data(), buffer.size(), hash);

    std::stringstream stream;
    for (auto &c : hash)
    {
        stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    checksum = stream.str();
}

toolkit::result<> unvm::Install(Config &config, http::HttpClient &client, std::string_view version, const VersionEntry &entry)
{
    if (config.Installed.contains(entry.Version))
    {
        std::cerr << "version '" << version << "' is already installed." << std::endl;
        return {};
    }

#if defined(SYSTEM_WINDOWS)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-win-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-win-arm64";
#endif

    constexpr auto extension = "zip";

#endif

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-linux-x64";
#endif

#if defined(ARCH_ARM64) || defined(ARCH_AARCH64)
    constexpr auto format = "node-{}-linux-arm64";
#endif

    constexpr auto extension = "tar.xz";

#endif

#if defined(SYSTEM_DARWIN)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-darwin-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-darwin-arm64";
#endif

    constexpr auto extension = "tar.xz";

#endif

    auto filename = std::format(format, entry.Version);
    auto pathname = std::format("/dist/{}/{}.{}", entry.Version, filename, extension);

    std::stringstream stream(std::stringstream::binary | std::stringstream::in | std::stringstream::out);

    http::HttpRequest request
    {
        .Method = http::HttpMethod::Get,
        .Location = {
            .UseTLS = true,
            .Host = "nodejs.org",
            .Port = 443,
            .Pathname = pathname,
        },
    };

    http::HttpResponse response
    {
        .Body = &stream,
    };

    if (auto res = client.Fetch(std::move(request), response); !res)
    {
        return toolkit::make_error("failed to get file: {}", res.error());
    }

    if (!http::IsSuccess(response.StatusCode))
    {
        return toolkit::make_error("failed to get file: {}, {}\n{}", response.StatusCode, response.StatusMessage, stream.str());
    }

    std::string checksum;
    if (auto res = get_checksum(client, entry, filename, extension, checksum); !res)
    {
        return toolkit::make_error("failed to get checksum: {}", res.error());
    }

    std::string archive_checksum;
    generate_checksum(stream, archive_checksum);

    if (archive_checksum != checksum)
    {
        return toolkit::make_error("checksum mismatch, archive checksum '{}' does not match '{}'.", archive_checksum, checksum);
    }

    auto data_directory = GetDataDirectory();

    if (std::error_code ec; std::filesystem::create_directories(data_directory, ec), ec)
    {
        return toolkit::make_error("failed to create directory '{}': {} ({}).", data_directory.string(), ec.message(), ec.value());
    }

    if (auto res = UnpackArchive(stream, data_directory); !res)
    {
        return toolkit::make_error("failed to unpack archive: {}", res.error());
    }

    auto from_path = data_directory / filename;
    auto to_path = data_directory / entry.Version;

    if (std::error_code ec; std::filesystem::rename(from_path, to_path, ec), ec)
    {
        return toolkit::make_error("failed to rename '{}' to '{}': {} ({})", from_path.string(), to_path.string(), ec.message(), ec.value());
    }

    config.Installed.emplace(entry.Version);
    config.Dirty = true;
    return {};
}

toolkit::result<> unvm::Install(Config &config, http::HttpClient &client, const std::string_view version)
{
    VersionTable table;
    if (auto res = LoadVersionTable(client, table, true); !res)
    {
        return toolkit::make_error("failed to load version table: {}", res.error());
    }

    const auto entry_ptr = FindEffectiveVersion(table, version);
    if (!entry_ptr)
    {
        return toolkit::make_error("no effective version for '{}'.", version);
    }

    return Install(config, client, version, *entry_ptr);
}
