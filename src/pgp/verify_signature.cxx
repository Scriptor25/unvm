#include <unvm/pgp.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>

#include <openssl/dsa.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

static toolkit::result<> verify_signature_md(
    unvm::pgp::HashAlgorithmID hash_algorithm,
    const std::span<const uint8_t> hash_left_16_bit,
    unvm::pgp::PublicKeyAlgorithmID public_key_algorithm,
    EVP_PKEY *public_key,
    const std::span<const uint8_t> signature,
    const std::span<const uint8_t> data)
{
    const EVP_MD *md;
    std::vector<uint8_t> em;

    switch (hash_algorithm)
    {
    case unvm::pgp::HashAlgorithmID::MD5:
        md = EVP_md5();
        em = { unvm::pgp::PREFIX_MD5.begin(), unvm::pgp::PREFIX_MD5.end() };
        break;

    case unvm::pgp::HashAlgorithmID::SHA1:
        md = EVP_sha1();
        em = { unvm::pgp::PREFIX_SHA1.begin(), unvm::pgp::PREFIX_SHA1.end() };
        break;

    case unvm::pgp::HashAlgorithmID::RIPEMD160:
        md = EVP_ripemd160();
        em = { unvm::pgp::PREFIX_RIPEMD160.begin(), unvm::pgp::PREFIX_RIPEMD160.end() };
        break;

    case unvm::pgp::HashAlgorithmID::SHA256:
        md = EVP_sha256();
        em = { unvm::pgp::PREFIX_SHA256.begin(), unvm::pgp::PREFIX_SHA256.end() };
        break;

    case unvm::pgp::HashAlgorithmID::SHA384:
        md = EVP_sha384();
        em = { unvm::pgp::PREFIX_SHA384.begin(), unvm::pgp::PREFIX_SHA384.end() };
        break;

    case unvm::pgp::HashAlgorithmID::SHA512:
        md = EVP_sha512();
        em = { unvm::pgp::PREFIX_SHA512.begin(), unvm::pgp::PREFIX_SHA512.end() };
        break;

    case unvm::pgp::HashAlgorithmID::SHA224:
        md = EVP_sha224();
        em = { unvm::pgp::PREFIX_SHA224.begin(), unvm::pgp::PREFIX_SHA224.end() };
        break;

    case unvm::pgp::HashAlgorithmID::SHA3_256:
        md = EVP_sha3_256();
        em = { unvm::pgp::PREFIX_SHA3_256.begin(), unvm::pgp::PREFIX_SHA3_256.end() };
        break;

    case unvm::pgp::HashAlgorithmID::SHA3_512:
        md = EVP_sha3_512();
        em = { unvm::pgp::PREFIX_SHA3_512.begin(), unvm::pgp::PREFIX_SHA3_512.end() };
        break;

    default:
        return toolkit::make_error("unsupported hash algorithm {}.", hash_algorithm);
    }

    auto *md_ctx = EVP_MD_CTX_new();
    if (!md_ctx)
    {
        return toolkit::make_error("failed to create md context: {}", unvm::GetSSLErrorStack());
    }

    auto guard_md_ctx = toolkit::defer(EVP_MD_CTX_free, md_ctx);

    // EVP_PKEY_CTX *pk_ctx;
    // if (EVP_DigestVerifyInit(md_ctx, &pk_ctx, md, nullptr, public_key) <= 0)
    // {
    //     return toolkit::make_error("failed to initialize verify digest: {}", unvm::GetSSLErrorStack());
    // }

    // if (EVP_DigestVerify(md_ctx, signature.data(), signature.size(), data.data(), data.size()) <= 0)
    // {
    //     return toolkit::make_error("failed to verify digest: {}", unvm::GetSSLErrorStack());
    // }

    if (EVP_DigestInit_ex(md_ctx, md, nullptr) <= 0)
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

    em.insert(em.end(), hash.begin(), hash.end());

    switch (public_key_algorithm)
    {
    case unvm::pgp::PublicKeyAlgorithmID::RSA_EO:
    case unvm::pgp::PublicKeyAlgorithmID::RSA_SO:
    case unvm::pgp::PublicKeyAlgorithmID::RSA_ES:
    {
        auto *rsa = EVP_PKEY_get1_RSA(public_key);
        if (!rsa)
        {
            return toolkit::make_error("failed to get rsa from public key: {}", unvm::GetSSLErrorStack());
        }

        auto guard_rsa = toolkit::defer(RSA_free, rsa);

        std::vector<uint8_t> decrypted(RSA_size(rsa));

        const auto decrypted_length = RSA_public_decrypt(
            static_cast<int>(signature.size()),
            signature.data(),
            decrypted.data(),
            rsa,
            RSA_PKCS1_PADDING);

        if (decrypted_length <= 0)
        {
            return toolkit::make_error("failed to decrypt using rsa public key: {}", unvm::GetSSLErrorStack());
        }

        decrypted.resize(decrypted_length);

        if (decrypted != em)
        {
            return toolkit::make_error("signature mismatch.");
        }

        return {};
    }

    default:
    {
        auto *ctx = EVP_MD_CTX_new();
        if (!ctx)
        {
            return toolkit::make_error("failed to create context: {}", unvm::GetSSLErrorStack());
        }

        auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

        if (public_key_algorithm == unvm::pgp::PublicKeyAlgorithmID::EdDSA)
        {
            md = nullptr;
        }

        EVP_PKEY_CTX *pctx;
        if (EVP_DigestVerifyInit(ctx, &pctx, md, nullptr, public_key) <= 0)
        {
            return toolkit::make_error("failed to initialize digest: {}", unvm::GetSSLErrorStack());
        }

        if (EVP_DigestVerify(ctx, signature.data(), signature.size(), data.data(), data.size()) <= 0)
        {
            return toolkit::make_error("failed to verify digest: {}", unvm::GetSSLErrorStack());
        }

        return {};
    }
    }
}

toolkit::result<> unvm::pgp::VerifySignature(
    const Signature &signature,
    const std::span<const uint8_t> data,
    EVP_PKEY *public_key)
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

    const auto sig = *signature.SignatureMaterial.begin();

    std::vector<uint8_t> sig_norm;
    if (auto res = NormalizeMPI(sig, EVP_PKEY_get_size(public_key)) >> sig_norm; !res)
    {
        return res;
    }

    return verify_signature_md(
        signature.HashAlgorithm,
        signature.HashLeft16Bit,
        signature.PublicKeyAlgorithm,
        public_key,
        sig_norm,
        buffer);
}
