#include <unvm/pgp.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>

static const std::unordered_map<unvm::pgp::HashAlgorithmID, const EVP_MD *> hash_algorithms
{
    { unvm::pgp::HashAlgorithmID::MD5, EVP_md5() },
    { unvm::pgp::HashAlgorithmID::SHA1, EVP_sha1() },
    { unvm::pgp::HashAlgorithmID::RIPEMD160, EVP_ripemd160() },
    { unvm::pgp::HashAlgorithmID::SHA256, EVP_sha256() },
    { unvm::pgp::HashAlgorithmID::SHA384, EVP_sha384() },
    { unvm::pgp::HashAlgorithmID::SHA512, EVP_sha512() },
    { unvm::pgp::HashAlgorithmID::SHA224, EVP_sha224() },
    { unvm::pgp::HashAlgorithmID::SHA3_256, EVP_sha3_256() },
    { unvm::pgp::HashAlgorithmID::SHA3_512, EVP_sha3_512() },
};

toolkit::result<> unvm::pgp::VerifySignature(
    const std::span<const uint8_t> buffer,
    const std::span<const uint8_t> signature_buffer,
    EVP_PKEY *public_key)
{
    auto *ptr = signature_buffer.data();
    auto *header = reinterpret_cast<const PacketHeader *>(ptr);

    PacketTypeID packet_type;
    uint32_t packet_length;
    uint8_t header_length;
    bool partial;

    header->Parse(packet_type, packet_length, header_length, partial);

    if (partial)
    {
        return toolkit::make_error("partial packets are not supported.");
    }

    if (packet_type != PacketTypeID::SignaturePacket)
    {
        return toolkit::make_error("unsupported packet type {}", packet_type);
    }

    auto *packet = reinterpret_cast<const SignaturePacket *>(ptr + header_length);

    Signature signature;
    if (auto res = ParseSignature(packet, packet_length) >> signature; !res)
    {
        return res;
    }

    const EVP_MD *md;
    if (const auto it = hash_algorithms.find(signature.HashAlgorithm); it != hash_algorithms.end())
    {
        md = it->second;
    }
    else
    {
        return toolkit::make_error("unsupported hash algorithm {}", signature.HashAlgorithm);
    }

    auto *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return toolkit::make_error("failed to create context: {}", GetSSLErrorStack());
    }

    auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, public_key) <= 0)
    {
        return toolkit::make_error("failed to initialize digest verify: {}", GetSSLErrorStack());
    }

    uint8_t length_length;
    switch (signature.Version)
    {
    case 0x04:
        // no salt required
        length_length = 2;
        break;

    case 0x06:
        length_length = 4;
        if (EVP_DigestVerifyUpdate(ctx, signature.SaltMaterial.data(), signature.SaltMaterial.size()) <= 0)
        {
            return toolkit::make_error("failed to update digest verify: {}", GetSSLErrorStack());
        }
        break;

    default:
        return toolkit::make_error("unsupported signature version {:02x}", signature.Version);
    }

    switch (signature.SignatureType)
    {
    case SignatureTypeID::Binary:
        if (EVP_DigestVerifyUpdate(ctx, buffer.data(), buffer.size()) <= 0)
        {
            return toolkit::make_error("failed to update digest verify: {}", GetSSLErrorStack());
        }
        break;

    default:
        return toolkit::make_error("unsupported signature type {}", signature.SignatureType);
    }

    const uint32_t signature_length = 4 + length_length + signature.HashedBlock.size();

    if (EVP_DigestVerifyUpdate(ctx, packet, signature_length) <= 0)
    {
        return toolkit::make_error("failed to update digest verify: {}", GetSSLErrorStack());
    }

    const uint8_t trailer[]
    {
        signature.Version,
        0xFF,
        static_cast<uint8_t>(signature_length >> 24 & 0xFF),
        static_cast<uint8_t>(signature_length >> 16 & 0xFF),
        static_cast<uint8_t>(signature_length >> 8 & 0xFF),
        static_cast<uint8_t>(signature_length & 0xFF),
    };

    if (EVP_DigestVerifyUpdate(ctx, trailer, sizeof(trailer)) <= 0)
    {
        return toolkit::make_error("failed to update digest verify: {}", GetSSLErrorStack());
    }

    const auto sig = *signature.SignatureMaterial.begin();

    if (EVP_DigestVerifyFinal(ctx, sig.data(), sig.size()) <= 0)
    {
        return toolkit::make_error("failed to finalize digest verify: {}", GetSSLErrorStack());
    }

    return {};
}
