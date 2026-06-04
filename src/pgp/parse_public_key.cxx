#include <unvm/pgp.hxx>

#include <toolkit/defer.hxx>

template<typename... A>
toolkit::result<std::vector<uint8_t>> digest(const EVP_MD *type, const A &... args)
{
    auto *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return toolkit::make_error("failed to create context.");
    }

    auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestInit(ctx, type) <= 0)
    {
        return toolkit::make_error("failed to initialize digest.");
    }

    auto res = (toolkit::result() & ... & [&]() -> toolkit::result<>
    {
        if constexpr (requires(A arg) { arg.data(); arg.size(); })
        {
            if (EVP_DigestUpdate(ctx, args.data(), args.size()) <= 0)
            {
                return toolkit::make_error("failed to update digest.");
            }
        }
        else
        {
            if (EVP_DigestUpdate(ctx, &args, sizeof(A)) <= 0)
            {
                return toolkit::make_error("failed to update digest.");
            }
        }

        return {};
    });

    if (!res)
    {
        return res;
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned hash_length = 0;

    if (EVP_DigestFinal_ex(ctx, hash, &hash_length) <= 0)
    {
        return toolkit::make_error("failed to finalize digest.");
    }

    return std::vector(hash, hash + hash_length);
}

toolkit::result<unvm::pgp::PublicKey> unvm::pgp::ParsePublicKey(const PublicKeyPacket *packet, uint32_t packet_length)
{
    PublicKey key;

    auto *pointer = reinterpret_cast<const uint8_t *>(packet);
    const std::span data(pointer, packet_length);

    switch (packet->Version)
    {
    case 0x04:
    {
        auto *pkg = reinterpret_cast<const PublicKeyPacketV4 *>(packet);

        key.CreationTime = scalar(pkg->CreationTime);
        key.Algorithm = pkg->PublicKeyAlgorithm;
        key.Material = { pkg->Key, pointer + packet_length };

        constexpr uint8_t magic = 0x99;

        if (auto res = digest(EVP_sha1(), magic, static_cast<uint16_t>(packet_length), data) >> key.Fingerprint; !res)
        {
            return res;
        }

        key.KeyID = { &key.Fingerprint[12], 8 };

        break;
    }
    case 0x06:
    {
        auto *pkg = reinterpret_cast<const PublicKeyPacketV6 *>(packet);

        key.CreationTime = scalar(pkg->CreationTime);
        key.Algorithm = pkg->PublicKeyAlgorithm;

        const auto key_length = scalar(pkg->KeyLength);
        key.Material = { pkg->Key, pkg->Key + key_length };

        constexpr uint8_t magic = 0x9B;

        if (auto res = digest(EVP_sha256(), magic, packet_length, data) >> key.Fingerprint; !res)
        {
            return res;
        }

        key.KeyID = { &key.Fingerprint[0], 8 };

        break;
    }
    default:
        return toolkit::make_error("unsupported packet version {:02x}", packet->Version);
    }

    return key;
}
