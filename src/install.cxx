#include <unvm/pgp.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>

#include <openssl/evp.h>
#include <openssl/pem.h>

#include <iostream>
#include <sstream>

static std::vector<unsigned char> base64_decode(const std::string &str)
{
    auto *bio = BIO_new_mem_buf(str.data(), static_cast<int>(str.size()));
    auto *base64 = BIO_new(BIO_f_base64());

    BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(base64, bio);

    std::vector<unsigned char> vec(str.size());

    const auto len = BIO_read(bio, vec.data(), static_cast<int>(vec.size()));
    vec.resize(len > 0 ? len : 0);

    BIO_free_all(bio);
    return vec;
}

EVP_PKEY *load_pubkey(const std::filesystem::path &filename)
{
    auto *file = fopen(filename.c_str(), "r");
    if (!file)
    {
        return nullptr;
    }

    auto *key = PEM_read_PUBKEY(file, nullptr, nullptr, nullptr);

    fclose(file);
    return key;
}

bool verify(EVP_PKEY *pubkey, const std::string &message, const std::vector<unsigned char> &signature)
{
    auto *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return false;
    }

    auto guard0 = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pubkey) != 1)
    {
        return false;
    }

    EVP_DigestVerifyUpdate(ctx, message.data(), message.size());

    if (EVP_DigestVerifyFinal(ctx, signature.data(), signature.size()) != 1)
    {
        return false;
    }

    return true;
}

bool shasums256_verify(std::istream &stream, EVP_PKEY *key)
{
    // const auto msg;
    // const auto sig;
    //
    // return verify(key, msg, sig);

    return false;
}

static toolkit::result<> get_file_from_repo(
    unvm::http::HttpClient &client,
    std::ostream &stream,
    std::string version,
    std::string filename)
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

    if (!unvm::http::IsSuccess(response.StatusCode))
    {
        return toolkit::make_error(
            "failed to get file {} (version {}) from repo: {}, {}",
            filename,
            version,
            response.StatusCode,
            response.StatusMessage);
    }

    return {};
}

static toolkit::result<std::string> get_checksum(
    unvm::http::HttpClient &client,
    const unvm::VersionEntry &entry,
    const std::string &filename,
    const std::string &extension)
{
    auto with_extension = filename + '.' + extension;

    std::stringstream stream(std::stringstream::in | std::stringstream::out);
    if (auto res = get_file_from_repo(client, stream, entry.Version, "SHASUMS256.txt"); !res)
    {
        return res;
    }

    std::stringstream sig_stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    if (auto res = get_file_from_repo(client, sig_stream, entry.Version, "SHASUMS256.txt.sig"); !res)
    {
        return res;
    }

    sig_stream.seekg(0, std::ios::end);
    std::vector<unsigned char> sig(sig_stream.tellg());
    sig_stream.seekg(0, std::ios::beg);
    sig_stream.read(reinterpret_cast<char *>(sig.data()), static_cast<std::streamsize>(sig.size()));

    {
        auto *header = reinterpret_cast<const unvm::pgp::PGPPacketHeader *>(sig.data());

        unvm::pgp::PacketTagValue packet_tag;
        uint8_t header_length;
        uint32_t packet_length;
        bool partial;

        header->Parse(packet_tag, packet_length, header_length, partial);

        if (partial)
        {
            (void) header_length;
            (void) packet_length;
            return toolkit::make_error("partial packets are not supported.");
        }

        if (packet_length == 0)
        {
            (void) header_length;
            return toolkit::make_error("indeterminate sized packets are not supported.");
        }

        if (packet_tag != unvm::pgp::PacketTagValue::SignaturePacket)
        {
            return toolkit::make_error(
                "invalid packet tag {:02X}, must be signature packet tag {:02X}.",
                static_cast<uint8_t>(packet_tag),
                static_cast<uint8_t>(unvm::pgp::PacketTagValue::SignaturePacket));
        }

        auto *body = reinterpret_cast<const uint8_t *>(header) + header_length;

        auto *packet = reinterpret_cast<const unvm::pgp::PGPVersion4SignaturePacket *>(body);
        if (packet->Version != 0x04)
        {
            return toolkit::make_error(
                "invalid packet version {:02X}, must be {:02X}",
                packet->Version,
                0x04);
        }

        if (packet->SignatureType != unvm::pgp::SignatureTypeValue::BinaryDocument)
        {
            return toolkit::make_error(
                "invalid signature type {:02X}, must be binary document signature type {:02X}.",
                static_cast<uint8_t>(packet->SignatureType),
                static_cast<uint8_t>(unvm::pgp::SignatureTypeValue::BinaryDocument));
        }

        auto *hashed_subpackets = reinterpret_cast<const unvm::pgp::PGPSubpacketSet *>(
            body + sizeof(unvm::pgp::PGPVersion4SignaturePacket)
        );

        auto *unhashed_subpackets = reinterpret_cast<const unvm::pgp::PGPSubpacketSet *>(
            body
            + sizeof(unvm::pgp::PGPVersion4SignaturePacket)
            + sizeof(unvm::pgp::PGPSubpacketSet)
            + hashed_subpackets->GetLength()
        );

        auto *signature = reinterpret_cast<const unvm::pgp::PGPSignatureBlock *>(
            body
            + sizeof(unvm::pgp::PGPVersion4SignaturePacket)
            + sizeof(unvm::pgp::PGPSubpacketSet)
            + hashed_subpackets->GetLength()
            + sizeof(unvm::pgp::PGPSubpacketSet)
            + unhashed_subpackets->GetLength()
        );

        for (auto descriptor : *hashed_subpackets)
        {
            (void) descriptor.Data->Type;

            std::cerr << descriptor.Length << std::endl;
        }

        for (auto descriptor : *unhashed_subpackets)
        {
            (void) descriptor.Data->Type;

            std::cerr << descriptor.Length << std::endl;
        }

        __builtin_debugtrap();
    }

    // TODO: verify pgp signature

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

static toolkit::result<std::string> generate_checksum(std::istream &str)
{
    auto *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return toolkit::make_error("failed to create evp context.");
    }

    auto guard0 = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1)
    {
        return toolkit::make_error("failed to initialize digest for sha256.");
    }

    std::array<char, 8192> chunk{};

    while (str)
    {
        str.read(chunk.data(), chunk.size());

        if (const auto count = str.gcount(); count > 0)
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

    std::ostringstream stream;
    for (unsigned i = 0; i < hash_length; ++i)
    {
        stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    str.clear();
    str.seekg(0, std::ios::beg);

    return stream.str();
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
    auto with_extension = std::format("{}.{}", filename, extension);

    std::stringstream stream(std::stringstream::binary | std::stringstream::in | std::stringstream::out);
    if (auto res = get_file_from_repo(client, stream, entry.Version, with_extension); !res)
    {
        return res;
    }

    std::string archive_checksum;
    if (auto res = generate_checksum(stream) >> archive_checksum; !res)
    {
        return toolkit::make_error("failed to generate checksum: {}", res.error());
    }

    std::string checksum;
    if (auto res = get_checksum(client, entry, filename, extension) >> checksum; !res)
    {
        return toolkit::make_error("failed to get checksum: {}", res.error());
    }

    if (archive_checksum != checksum)
    {
        return toolkit::make_error(
            "checksum mismatch, archive checksum '{}' does not match '{}'.",
            archive_checksum,
            checksum);
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

    if (auto res = UnpackArchive(stream, data_directory); !res)
    {
        return toolkit::make_error("failed to unpack archive: {}", res.error());
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
