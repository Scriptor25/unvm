#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <openssl/sha.h>

#include <iostream>

static int get_checksum(unvm::http::Client &client, const unvm::VersionEntry &entry, const std::string &filename, const std::string &extension, std::string &checksum)
{
    auto with_extension = filename + '.' + extension;

    auto pathname = std::format("/dist/{}/SHASUMS256.txt", entry.Version);

    std::stringstream stream(std::stringstream::in | std::stringstream::out);
    
    unvm::http::Request request
    {
        .Method = unvm::http::Method::Get,
        .Location = {
            .UseTLS = true,
            .Host = "nodejs.org",
            .Port = 443,
            .Pathname = pathname,
        },
    };

    unvm::http::Response response
    {
        .Body = &stream,
    };

    if (auto error = client.Fetch(std::move(request), response))
    {
        std::cerr << "failed to get file." << std::endl;
        return error;
    }

    if (!unvm::http::IsSuccess(response.StatusCode))
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

    for (std::string line; std::getline(stream, line); )
    {
        line = unvm::Trim(std::move(line));
        if (line.empty())
        {
            continue;
        }

        if (auto pos = line.find(with_extension); pos != std::string::npos)
        {
            checksum = unvm::Trim(line.substr(0, pos));
            return 0;
        }
    }

    std::cerr << "failed to get checksum for filename '" << with_extension << "'." << std::endl;
    return 1;
}

static int generate_checksum(std::istream &str, std::string &checksum)
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

    return 0;
}

int unvm::Install(Config &config, http::Client &client, std::string_view version, const VersionEntry &entry)
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

    http::Request request
    {
        .Method = http::Method::Get,
        .Location = {
            .UseTLS = true,
            .Host = "nodejs.org",
            .Port = 443,
            .Pathname = pathname,
        },
    };

    http::Response response
    {
        .Body = &stream,
    };

    if (auto error = client.Fetch(std::move(request), response))
    {
        std::cerr << "failed to get file." << std::endl;
        return error;
    }

    if (!http::IsSuccess(response.StatusCode))
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

    std::string checksum;
    if (const auto error = get_checksum(client, entry, filename, extension, checksum))
    {
        std::cerr << "failed to get checksum." << std::endl;
        return error;
    }

    std::string archive_checksum;
    if (const auto error = generate_checksum(stream, archive_checksum))
    {
        std::cerr << "failed to generate archive checksum." << std::endl;
        return error;
    }

    if (archive_checksum != checksum)
    {
        std::cerr << "checksum mismatch, archive checksum '" << archive_checksum << "' does not match '" << checksum << "'." << std::endl;
        return 1;
    }

    if (std::error_code ec; std::filesystem::create_directories(config.InstallDirectory, ec), ec)
    {
        std::cerr
                << "failed to create install directory '"
                << config.InstallDirectory.string()
                << "': "
                << ec.message()
                << " ("
                << ec.value()
                << ")"
                << std::endl;
        return ec.value();
    }

    if (const auto error = UnpackArchive(stream, config.InstallDirectory))
    {
        std::cerr << "failed to unpack archive." << std::endl;
        return error;
    }

    if (std::error_code ec; std::filesystem::rename(config.InstallDirectory / filename, config.InstallDirectory / entry.Version, ec), ec)
    {
        std::cerr
                << "failed to rename '"
                << (config.InstallDirectory / filename).string()
                << "' to '"
                << (config.InstallDirectory / entry.Version).string()
                << "': "
                << ec.message()
                << " ("
                << ec.value()
                << ")"
                << std::endl;
        return ec.value();
    }

    config.Installed.emplace(entry.Version);
    return 0;
}

int unvm::Install(Config &config, http::Client &client, std::string_view version)
{
    VersionTable table;
    if (auto error = LoadVersionTable(client, table, true))
    {
        std::cerr << "failed to load version table." << std::endl;
        return error;
    }

    auto entry_ptr = FindEffectiveVersion(table, version);
    if (!entry_ptr)
    {
        std::cerr << "no effective version for '" << version << "'." << std::endl;
        return 1;
    }

    return Install(config, client, version, *entry_ptr);
}
