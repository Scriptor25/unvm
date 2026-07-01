#include <unvm/data.hxx>
#include <unvm/lock.hxx>
#include <unvm/pgp.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>

#include <openssl/evp.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

[[nodiscard]] static toolkit::result<bool> get_file_from_repo(
    http::Client &client,
    std::ostream &stream,
    std::string version,
    std::string filename,
    const bool optional)
{
    http::HttpRequest request
    {
        .Method = http::HttpMethod::Get,
        .Location = {
            .Scheme = "https",
            .Host = "nodejs.org",
            .Port = 443,
            .Pathname = std::format("/dist/{}/{}", version, filename),
        },
    };

    http::HttpResponse response
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

    if (optional && response.StatusCode == http::HttpStatusCode::NotFound)
    {
        return false;
    }

    if (!http::IsSuccess(response.StatusCode))
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

[[nodiscard]] static toolkit::result<std::string> get_trusted_checksum(
    unvm::Config &config,
    http::Client &client,
    const unvm::VersionEntry &entry,
    const std::string &with_extension)
{
    constexpr auto flags = std::stringstream::in | std::stringstream::out | std::stringstream::binary;

    std::stringstream stream(flags);
    if (auto res = get_file_from_repo(
        client,
        stream,
        entry.Version,
        "SHASUMS256.txt",
        false
    ); !res)
    {
        return res;
    }

    bool has_signature;
    std::stringstream signature_stream(flags);
    if (auto res = get_file_from_repo(
                       client,
                       signature_stream,
                       entry.Version,
                       "SHASUMS256.txt.sig",
                       true
                   ) >> has_signature; !res)
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
            return toolkit::make_error("failed to parse keyring: {}", res.error());
        }

        unvm::pgp::Signature signature;
        if (auto res = unvm::pgp::ParseSignature(signature_buffer) >> signature; !res)
        {
            return toolkit::make_error("failed to parse signature: {}", res.error());
        }

        if (auto *key = unvm::pgp::MatchPublicKey(
            keyring,
            signature,
            static_cast<uint8_t>(unvm::pgp::KeyUsageFlag::Sign)))
        {
            EVP_PKEY *public_key{};
            if (auto res = unvm::pgp::CreateOpenSSLPublicKey(*key) >> public_key; !res)
            {
                return toolkit::make_error("failed to create public key: {}", res.error());
            }

            if (auto res = unvm::pgp::VerifySignature(
                signature,
                data_buffer,
                public_key,
                EVP_PKEY_get_size(public_key)); !res)
            {
                return toolkit::make_error("failed to verify signature: {}", res.error());
            }
        }
        else
        {
            if (auto fingerprint = unvm::pgp::ToHexString(signature.IssuerFingerprint);
                !config.Fingerprints.contains(fingerprint))
            {
                std::cout << "untrusted fingerprint '" << fingerprint << "'." << std::endl;

                if (auto trust = unvm::Confirm("trust this fingerprint?"); !trust)
                {
                    return toolkit::make_error("untrusted fingerprint '{}'.", fingerprint);
                }

                config.Fingerprints.insert(fingerprint);
                config.AddedFingerprints.insert(fingerprint);
            }
        }
    }
    else
    {
        std::cout << "version '" << entry.Version << "' does not have a signature file." << std::endl;

        if (auto trust = unvm::Confirm("install anyways?"); !trust)
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

[[nodiscard]] static toolkit::result<std::string> get_file_checksum(std::istream &stream)
{
    auto *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return toolkit::make_error("failed to create context: {}", unvm::GetSSLErrorStack());
    }

    auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestInit(ctx, EVP_sha256()) <= 0)
    {
        return toolkit::make_error("failed to initialize context: {}", unvm::GetSSLErrorStack());
    }

    std::array<char, 8192> chunk{};

    while (stream)
    {
        stream.read(chunk.data(), chunk.size());

        if (const auto count = stream.gcount(); count > 0)
        {
            if (EVP_DigestUpdate(ctx, chunk.data(), count) <= 0)
            {
                return toolkit::make_error("failed to update context: {}", unvm::GetSSLErrorStack());
            }
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned hash_length = 0;

    if (EVP_DigestFinal(ctx, hash, &hash_length) <= 0)
    {
        return toolkit::make_error("failed to finalize context: {}", unvm::GetSSLErrorStack());
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
    http::Client &client,
    std::string_view version,
    const VersionEntry &entry)
{
    if (config.Installed.contains(entry.Version))
    {
        std::cerr << "version '" << version << "' is already installed." << std::endl;
        return {};
    }

    auto [format, extension, pattern] = platform;

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

    config.Installed.insert(entry.Version);
    config.AddedVersions.insert(entry.Version);
    return {};
}

toolkit::result<> unvm::Install(Config &config, http::Client &client, const std::string_view version)
{
    VersionTable table;
    if (auto res = LoadVersionTable(client, table, true); !res)
    {
        return toolkit::make_error("failed to load version table: {}", res.error());
    }

    FilterVersionTable(config, table, true);

    const VersionEntry *entry{};
    if (auto res = FindVersionEntry(table, version) >> entry; !res)
    {
        return res;
    }

    if (!entry)
    {
        return toolkit::make_error("no version matching '{}'.", version);
    }

    const auto data_directory = GetDataDirectory();
    const auto lock_path = data_directory / (entry->Version + ".lock");

    TryAcquire lock(lock_path, false, "install");
    if (!lock)
    {
        if (lock.Message() == "install")
        {
            std::cout << "version '" << version << "' is already being installed by another process." << std::endl;
            return {};
        }

        lock = TryAcquire(lock_path, true, "install");

        if (auto res = ReloadConfigFile(config); !res)
        {
            return res;
        }
    }

    (void) lock;

    return Install(config, client, version, *entry);
}
