#include <unvm/data.hxx>
#include <unvm/pgp.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>
#include <toolkit/string.hxx>

#include <openssl/evp.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

static toolkit::result<bool> get_file_from_repo(
    unvm::http::HttpClient &client,
    std::ostream &stream,
    std::string version,
    std::string filename,
    bool optional)
{
    unvm::http::HttpRequest request
    {
        .Method = unvm::http::HttpMethod::Get,
        .Location = {
            .UseTLS = true,
            .Host = "nodejs.org",
            .Port = 443,
            .Pathname = std::format("/dist/{}/{}", version, filename),
        },
    };

    unvm::http::HttpResponse response
    {
        .Body = &stream,
    };

    if (auto res = client.FetchWithRedirects(std::move(request), response); !res)
    {
        return toolkit::make_error(
            "failed to get file {} (version {}) from repo: {}",
            filename,
            version,
            res.error());
    }

    if (optional && response.StatusCode == unvm::http::HttpStatusCode::NotFound)
    {
        return false;
    }

    if (!unvm::http::IsSuccess(response.StatusCode))
    {
        return toolkit::make_error(
            "failed to get file {} (version {}) from repo: {}, {}",
            filename,
            version,
            response.StatusCode,
            response.StatusMessage);
    }

    return true;
}

template<typename... M>
std::string prompt(const M &... message)
{
    (std::cout << ... << message) << std::flush;

    std::string input;
    std::getline(std::cin, input);

    return input;
}

template<typename... M>
static bool confirm(const M &... message)
{
    auto input = toolkit::lowercase(prompt(message..., " [y/N]: "));

    return input == "y" || input == "yes";
}

static toolkit::result<std::string> get_trusted_checksum(
    unvm::Config &config,
    unvm::http::HttpClient &client,
    const unvm::VersionEntry &entry,
    const std::string &with_extension)
{
    std::stringstream stream(std::stringstream::in | std::stringstream::out);
    if (auto res = get_file_from_repo(client, stream, entry.Version, "SHASUMS256.txt", false); !res)
    {
        return res;
    }

    std::stringstream signature_stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    bool has_signature;
    if (auto res = get_file_from_repo(
                       client,
                       signature_stream,
                       entry.Version,
                       "SHASUMS256.txt.sig",
                       true) >> has_signature; !res)
    {
        return res;
    }

    if (has_signature)
    {
        const auto data_string = stream.str();
        const auto signature_string = signature_stream.str();

        const std::vector<uint8_t> data_buffer(data_string.begin(), data_string.end());
        const std::vector<uint8_t> signature_buffer(signature_string.begin(), signature_string.end());

        unvm::pgp::Keyring keyring;
        if (auto res = unvm::pgp::ParseKeyring(unvm::data::keyring) >> keyring; !res)
        {
            return res;
        }

        // TODO: extract fingerprint from signature buffer
        // TODO: extract public key from trusted key store using fingerprint
        // TODO: if no key is found, ask user for confirmation

        // if (!config.Fingerprints.contains(fingerprint))
        // {
        //     std::cout << "untrusted fingerprint '" << fingerprint << "'." << std::endl;
        //
        //     if (auto trust = confirm("trust this fingerprint?"); !trust)
        //     {
        //         return toolkit::make_error("untrusted fingerprint '{}'.", fingerprint);
        //     }
        //
        //     config.Fingerprints.insert(fingerprint);
        //     config.Dirty = true;
        // }

        // TODO: else verify signature

        if (auto res = unvm::pgp::VerifySignature(data_buffer, signature_buffer, nullptr); !res)
        {
            return res;
        }
    }
    else
    {
        std::cout << "version '" << entry.Version << "' does not have a signature file." << std::endl;

        if (auto trust = confirm("install anyways?"); !trust)
        {
            return toolkit::make_error("version '{}' does not have a signature file.", entry.Version);
        }
    }

    for (std::string line; std::getline(stream, line);)
    {
        std::istringstream str(line);

        if (std::string hash, file; (str >> hash >> file) && file == with_extension)
        {
            return hash;
        }
    }

    return toolkit::make_error("failed to get checksum for filename '{}'.", with_extension);
}

static toolkit::result<std::string> get_file_checksum(std::istream &stream)
{
    auto *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return toolkit::make_error("failed to create evp context.");
    }

    auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1)
    {
        return toolkit::make_error("failed to initialize digest for sha256.");
    }

    std::array<char, 8192> chunk{};

    while (stream)
    {
        stream.read(chunk.data(), chunk.size());

        if (const auto count = stream.gcount(); count > 0)
        {
            if (EVP_DigestUpdate(ctx, chunk.data(), count) != 1)
            {
                return toolkit::make_error("failed to update digest.");
            }
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned hash_length = 0;

    if (EVP_DigestFinal_ex(ctx, hash, &hash_length) != 1)
    {
        return toolkit::make_error("failed to finalize digest.");
    }

    std::ostringstream os;
    for (unsigned i = 0; i < hash_length; ++i)
    {
        os << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(hash[i]);
    }

    return os.str();
}

toolkit::result<> unvm::Install(
    Config &config,
    http::HttpClient &client,
    std::string_view version,
    const VersionEntry &entry)
{
    if (config.Installed.contains(entry.Version))
    {
        std::cerr << "version '" << version << "' is already installed." << std::endl;
        return {};
    }

#if defined(SYSTEM_WINDOWS)

#if defined(ARCH_X86)
    constexpr auto format = "node-{}-win-x86";
#endif

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-win-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-win-arm64";
#endif

    constexpr auto extension = "zip";

#endif

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID)

#if defined(ARCH_X86)
    constexpr auto format = "node-{}-linux-x86";
#endif

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-linux-x64";
#endif

#if defined(ARCH_ARM64) || defined(ARCH_AARCH64)
    constexpr auto format = "node-{}-linux-arm64";
#endif

    constexpr auto extension = "tar.gz";

#endif

#if defined(SYSTEM_DARWIN)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-darwin-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-darwin-arm64";
#endif

    constexpr auto extension = "tar.gz";

#endif

    auto filename = std::format(format, entry.Version);
    auto with_extension = std::format("{}.{}", filename, extension);

    std::string trusted_checksum;
    if (auto res = get_trusted_checksum(config, client, entry, with_extension) >> trusted_checksum; !res)
    {
        return toolkit::make_error("failed to get trusted checksum: {}", res.error());
    }

    char archive_name[L_tmpnam];
    tmpnam(archive_name);

    auto guard_archive = toolkit::defer(
        [](const char *name)
        {
            if (std::error_code ec; std::filesystem::remove(name, ec), ec)
            {
                std::cerr
                        << "failed to remove file '"
                        << name
                        << "': "
                        << ec.message()
                        << " ("
                        << ec.value()
                        << ")."
                        << std::endl;
            }
        },
        archive_name);

    // write archive to disk
    {
        std::ofstream archive_stream(archive_name, std::ios::binary);
        if (auto res = get_file_from_repo(client, archive_stream, entry.Version, with_extension, false); !res)
        {
            return toolkit::make_error("failed to get archive: {}", res.error());
        }
    }

    // read archive from disk, get file checksum
    std::string archive_checksum;
    {
        std::ifstream archive_stream(archive_name, std::ios::binary);
        if (auto res = get_file_checksum(archive_stream) >> archive_checksum; !res)
        {
            return toolkit::make_error("failed to generate archive checksum: {}", res.error());
        }
    }

    if (archive_checksum != trusted_checksum)
    {
        return toolkit::make_error(
            "checksum mismatch, archive checksum '{}' does not match trusted checksum '{}'.",
            archive_checksum,
            trusted_checksum);
    }

    auto data_directory = GetDataDirectory();

    if (std::error_code ec; std::filesystem::create_directories(data_directory, ec), ec)
    {
        return toolkit::make_error(
            "failed to create directory '{}': {} ({}).",
            data_directory.string(),
            ec.message(),
            ec.value());
    }

    // read archive from disk, unpack archive
    {
        std::ifstream archive_stream(archive_name, std::ios::binary);
        if (auto res = UnpackArchive(archive_stream, data_directory); !res)
        {
            return toolkit::make_error("failed to unpack archive: {}", res.error());
        }
    }

    auto from_path = data_directory / filename;
    auto to_path = data_directory / entry.Version;

    if (std::error_code ec; std::filesystem::rename(from_path, to_path, ec), ec)
    {
        return toolkit::make_error(
            "failed to rename '{}' to '{}': {} ({})",
            from_path.string(),
            to_path.string(),
            ec.message(),
            ec.value());
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
