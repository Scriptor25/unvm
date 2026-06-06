#include <unvm/pgp.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>

#include <openssl/dsa.h>
#include <openssl/ec.h>
#include <openssl/evp.h>

#include <iostream>

static toolkit::result<> verify_signature(
    const unvm::pgp::HashAlgorithmID hash_algorithm,
    const std::span<const uint8_t> hash_left_16_bit,
    const unvm::pgp::PublicKeyAlgorithmID public_key_algorithm,
    EVP_PKEY *public_key,
    const std::span<const uint8_t> signature,
    const std::span<const uint8_t> data)
{
    const EVP_MD *digest;
    std::span<const uint8_t> prefix;

    switch (hash_algorithm)
    {
    case unvm::pgp::HashAlgorithmID::MD5:
        digest = EVP_md5();
        prefix = unvm::pgp::PREFIX_MD5;
        break;

    case unvm::pgp::HashAlgorithmID::SHA1:
        digest = EVP_sha1();
        prefix = unvm::pgp::PREFIX_SHA1;
        break;

    case unvm::pgp::HashAlgorithmID::RIPEMD160:
        digest = EVP_ripemd160();
        prefix = unvm::pgp::PREFIX_RIPEMD160;
        break;

    case unvm::pgp::HashAlgorithmID::SHA256:
        digest = EVP_sha256();
        prefix = unvm::pgp::PREFIX_SHA256;
        break;

    case unvm::pgp::HashAlgorithmID::SHA384:
        digest = EVP_sha384();
        prefix = unvm::pgp::PREFIX_SHA384;
        break;

    case unvm::pgp::HashAlgorithmID::SHA512:
        digest = EVP_sha512();
        prefix = unvm::pgp::PREFIX_SHA512;
        break;

    case unvm::pgp::HashAlgorithmID::SHA224:
        digest = EVP_sha224();
        prefix = unvm::pgp::PREFIX_SHA224;
        break;

    case unvm::pgp::HashAlgorithmID::SHA3_256:
        digest = EVP_sha3_256();
        prefix = unvm::pgp::PREFIX_SHA3_256;
        break;

    case unvm::pgp::HashAlgorithmID::SHA3_512:
        digest = EVP_sha3_512();
        prefix = unvm::pgp::PREFIX_SHA3_512;
        break;

    default:
        return toolkit::make_error("unsupported hash algorithm {}.", hash_algorithm);
    }

    (void) prefix;

    auto *md_ctx = EVP_MD_CTX_new();
    if (!md_ctx)
    {
        return toolkit::make_error("failed to create md context: {}", unvm::GetSSLErrorStack());
    }

    auto guard_md_ctx = toolkit::defer(EVP_MD_CTX_free, md_ctx);

    if (EVP_DigestInit_ex(md_ctx, digest, nullptr) <= 0)
    {
        return toolkit::make_error("failed to initialize digest: {}", unvm::GetSSLErrorStack());
    }

    if (EVP_DigestUpdate(md_ctx, data.data(), data.size()) <= 0)
    {
        return toolkit::make_error("failed to update digest: {}", unvm::GetSSLErrorStack());
    }

    unsigned char hash_buffer[EVP_MAX_MD_SIZE];
    unsigned int hash_length;

    if (EVP_DigestFinal_ex(md_ctx, hash_buffer, &hash_length) <= 0)
    {
        return toolkit::make_error("failed to finalize digest: {}", unvm::GetSSLErrorStack());
    }

    const std::span hash(hash_buffer, hash_length);

    if (hash_left_16_bit != hash.subspan(0, 2))
    {
        return toolkit::make_error(
            "hash left 16 bit do not match: {}, {}.",
            unvm::pgp::ToHexString(hash_left_16_bit),
            unvm::pgp::ToHexString(hash));
    }

    if (public_key_algorithm == unvm::pgp::PublicKeyAlgorithmID::EdDSALegacy
        || public_key_algorithm == unvm::pgp::PublicKeyAlgorithmID::Ed25519
        || public_key_algorithm == unvm::pgp::PublicKeyAlgorithmID::Ed448)
    {
        if (auto type = EVP_PKEY_id(public_key); type != EVP_PKEY_ED25519 && type != EVP_PKEY_ED448)
        {
            return toolkit::make_error("invalid public key, is not EdDSA (type: {}).", type);
        }

        auto *ctx = EVP_PKEY_CTX_new(public_key, nullptr);
        if (!ctx)
        {
            return toolkit::make_error("failed to create public key context: {}", unvm::GetSSLErrorStack());
        }

        auto guard_ctx = toolkit::defer(EVP_PKEY_CTX_free, ctx);

        if (EVP_PKEY_verify_init(ctx) <= 0)
        {
            return toolkit::make_error("failed to initialize public key context: {}", unvm::GetSSLErrorStack());
        }

        if (EVP_PKEY_verify(ctx, signature.data(), signature.size(), data.data(), data.size()) <= 0)
        {
            return toolkit::make_error("failed to verify public key: {}", unvm::GetSSLErrorStack());
        }
    }
    else
    {
        auto *ctx = EVP_MD_CTX_new();
        if (!ctx)
        {
            return toolkit::make_error("failed to create context: {}", unvm::GetSSLErrorStack());
        }

        auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

        if (EVP_DigestVerifyInit(ctx, nullptr, digest, nullptr, public_key) <= 0)
        {
            return toolkit::make_error("failed to initialize digest: {}", unvm::GetSSLErrorStack());
        }

        if (EVP_DigestVerifyUpdate(ctx, data.data(), data.size()) <= 0)
        {
            return toolkit::make_error("failed to update verify digest: {}", unvm::GetSSLErrorStack());
        }

        if (EVP_DigestVerifyFinal(ctx, signature.data(), signature.size()) <= 0)
        {
            return toolkit::make_error("failed to finalize verify digest: {}", unvm::GetSSLErrorStack());
        }
    }

    return {};
}

toolkit::result<> unvm::pgp::VerifySignature(
    const Signature &signature,
    const std::span<const uint8_t> data,
    EVP_PKEY *public_key,
    const size_t public_key_size)
{
    std::vector<uint8_t> buffer;
    BuildSignatureBuffer(signature, data) >> buffer;

    return VerifySignatureBuffer(signature, buffer, public_key, public_key_size);
}

toolkit::result<std::vector<uint8_t>> unvm::pgp::BuildSignatureBuffer(
    const Signature &signature,
    const std::span<const uint8_t> data)
{
    std::vector<uint8_t> buffer;
    auto update = [&buffer](std::span<const uint8_t> block)
    {
        buffer.insert(buffer.end(), block.begin(), block.end());
    };

    uint8_t length_length;
    switch (signature.Version)
    {
    case 0x04:
        // no salt required
        length_length = 2;
        break;

    case 0x06:
        update(signature.SaltMaterial);
        length_length = 4;
        break;

    default:
        return toolkit::make_error("unsupported signature version {:02x}", signature.Version);
    }

    switch (signature.SignatureType)
    {
    case SignatureTypeID::Binary:
        update(data);
        break;

    default:
        return toolkit::make_error("unsupported signature type {}", signature.SignatureType);
    }

    update(
        std::array
        {
            signature.Version,
            static_cast<uint8_t>(signature.SignatureType),
            static_cast<uint8_t>(signature.PublicKeyAlgorithm),
            static_cast<uint8_t>(signature.HashAlgorithm),
        });

    switch (signature.Version)
    {
    case 0x04:
        update(bytes<uint16_t>(signature.HashedBlock.size()));
        break;

    case 0x06:
        update(bytes<uint32_t>(signature.HashedBlock.size()));
        break;

    default:
        return toolkit::make_error("unsupported signature version {:02x}", signature.Version);
    }

    update(signature.HashedBlock.block);

    update(
        std::array
        {
            signature.Version,
            static_cast<uint8_t>(0xFF)
        });

    const auto signature_length = 4 + length_length + signature.HashedBlock.size();
    update(bytes<uint32_t>(signature_length));

    return buffer;
}

toolkit::result<> unvm::pgp::VerifySignatureBuffer(
    const Signature &signature,
    std::span<const uint8_t> buffer,
    EVP_PKEY *public_key,
    size_t public_key_size)
{
    std::vector<uint8_t> sig;

    auto mpi = signature.SignatureMaterial.begin();

    switch (signature.PublicKeyAlgorithm)
    {
    case PublicKeyAlgorithmID::RSA_EO:
    case PublicKeyAlgorithmID::RSA_SO:
    case PublicKeyAlgorithmID::RSA_ES:
    {
        const auto raw = *mpi++;

        if (auto res = NormalizeMPI(raw, public_key_size) >> sig; !res)
        {
            return res;
        }

        break;
    }

    case PublicKeyAlgorithmID::DSA:
    {
        const auto raw_r = *mpi++;
        const auto raw_s = *mpi++;

        auto *r = BN_bin2bn(raw_r.data(), raw_r.size(), nullptr);
        auto *s = BN_bin2bn(raw_s.data(), raw_s.size(), nullptr);

        auto guard_r = toolkit::defer(BN_free, r);
        auto guard_s = toolkit::defer(BN_free, s);

        auto *dsa_sig = DSA_SIG_new();
        if (!dsa_sig)
        {
            return toolkit::make_error("failed to create dsa signature: {}", GetSSLErrorStack());
        }

        auto guard_dsa_sig = toolkit::defer(DSA_SIG_free, dsa_sig);

        if (DSA_SIG_set0(dsa_sig, r, s) <= 0)
        {
            return toolkit::make_error("failed to set dsa signature: {}", GetSSLErrorStack());
        }

        guard_r.deactivate();
        guard_s.deactivate();

        auto der_size = i2d_DSA_SIG(dsa_sig, nullptr);
        if (der_size <= 0)
        {
            return toolkit::make_error("failed to get dsa signature der size: {}", GetSSLErrorStack());
        }

        std::vector<uint8_t> der(der_size);

        auto *out = der.data();
        der_size = i2d_DSA_SIG(dsa_sig, &out);
        if (der_size <= 0)
        {
            return toolkit::make_error("failed to encode dsa signature der: {}", GetSSLErrorStack());
        }

        sig = std::move(der);
        break;
    }

    case PublicKeyAlgorithmID::ECDSA:
    {
        const auto raw_r = *mpi++;
        const auto raw_s = *mpi++;

        auto *r = BN_bin2bn(raw_r.data(), raw_r.size(), nullptr);
        auto *s = BN_bin2bn(raw_s.data(), raw_s.size(), nullptr);

        auto guard_r = toolkit::defer(BN_free, r);
        auto guard_s = toolkit::defer(BN_free, s);

        auto *ecdsa_sig = ECDSA_SIG_new();
        if (!ecdsa_sig)
        {
            return toolkit::make_error("failed to create ecdsa signature: {}", GetSSLErrorStack());
        }

        auto guard_dsa_sig = toolkit::defer(ECDSA_SIG_free, ecdsa_sig);

        if (ECDSA_SIG_set0(ecdsa_sig, r, s) <= 0)
        {
            return toolkit::make_error("failed to set ecdsa signature: {}", GetSSLErrorStack());
        }

        guard_r.deactivate();
        guard_s.deactivate();

        auto der_size = i2d_ECDSA_SIG(ecdsa_sig, nullptr);
        if (der_size <= 0)
        {
            return toolkit::make_error("failed to get ecdsa signature der size: {}", GetSSLErrorStack());
        }

        std::vector<uint8_t> der(der_size);

        auto *out = der.data();
        der_size = i2d_ECDSA_SIG(ecdsa_sig, &out);
        if (der_size <= 0)
        {
            return toolkit::make_error("failed to encode ecdsa signature der: {}", GetSSLErrorStack());
        }

        sig = std::move(der);
        break;
    }

    case PublicKeyAlgorithmID::EdDSALegacy:
    {
        const auto raw_r = *mpi++;
        const auto raw_s = *mpi++;

        sig.insert(sig.end(), raw_r.rbegin(), raw_r.rend());
        sig.insert(sig.end(), raw_s.rbegin(), raw_s.rend());
        break;
    }

    case PublicKeyAlgorithmID::Ed25519:
    {
        const auto raw = mpi.bytes(64);

        sig = { raw.begin(), raw.end() };
        break;
    }

    case PublicKeyAlgorithmID::Ed448:
    {
        const auto raw = mpi.bytes(114);

        sig = { raw.begin(), raw.end() };
        break;
    }

    default:
        return toolkit::make_error("unsupported public key algorithm {}.", signature.PublicKeyAlgorithm);
    }

    return verify_signature(
        signature.HashAlgorithm,
        signature.HashLeft16Bit,
        signature.PublicKeyAlgorithm,
        public_key,
        sig,
        buffer);
}
