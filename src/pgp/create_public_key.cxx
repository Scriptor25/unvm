#include <unvm/pgp.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>
#include <toolkit/result.hxx>

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>

#include <cstring>

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_RSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto n = *cursor++;
    auto e = *cursor++;

    auto *ctx = EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr);
    if (!ctx)
    {
        return toolkit::make_error("failed to create context: {}", GetSSLErrorStack());
    }

    auto guard_ctx = toolkit::defer(EVP_PKEY_CTX_free, ctx);

    if (EVP_PKEY_fromdata_init(ctx) <= 0)
    {
        return toolkit::make_error("failed to initialize public key from data: {}", GetSSLErrorStack());
    }

    OSSL_PARAM params[]
    {
        OSSL_PARAM_BN("n", const_cast<uint8_t *>(n.data()), n.size()),
        OSSL_PARAM_BN("e", const_cast<uint8_t *>(e.data()), e.size()),
        OSSL_PARAM_END,
    };

    EVP_PKEY *public_key{};
    if (EVP_PKEY_fromdata(ctx, &public_key, EVP_PKEY_PUBLIC_KEY, params) <= 0)
    {
        return toolkit::make_error("failed to create public key from data: {}", GetSSLErrorStack());
    }

    return public_key;
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_EC(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    const auto curve = cursor.curve();
    auto *group = ToString(curve);

    auto q = *cursor++;

    auto *name = (curve == CurveOID::Curve25519 || curve == CurveOID::Ed25519) ? group : "EC";

    auto *ctx = EVP_PKEY_CTX_new_from_name(nullptr, name, nullptr);
    if (!ctx)
    {
        return toolkit::make_error("failed to create context: {}", GetSSLErrorStack());
    }

    auto guard_ctx = toolkit::defer(EVP_PKEY_CTX_free, ctx);

    if (EVP_PKEY_fromdata_init(ctx) <= 0)
    {
        return toolkit::make_error("failed to initialize public key from data: {}", GetSSLErrorStack());
    }

    OSSL_PARAM params[3];
    if (curve == CurveOID::Curve25519 || curve == CurveOID::Ed25519)
    {
        if (!q.empty() && q[0] == 0x40)
        {
            q = q.subspan(1);
        }

        if (q.size() != 0x20)
        {
            return toolkit::make_error("invalid key size: {}", q.size());
        }

        params[0] = OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size());
        params[1] = OSSL_PARAM_END;
    }
    else
    {
        params[0] = OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, const_cast<char *>(group), strlen(group));
        params[1] = OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size());
        params[2] = OSSL_PARAM_END;
    }

    EVP_PKEY *public_key{};
    if (EVP_PKEY_fromdata(ctx, &public_key, EVP_PKEY_PUBLIC_KEY, params) <= 0)
    {
        return toolkit::make_error("failed to create public key from data: {}", GetSSLErrorStack());
    }

    return public_key;
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_EdDSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    const auto curve = cursor.curve();
    auto *group = ToString(curve);

    auto q = *cursor++;

    auto *ctx = EVP_PKEY_CTX_new_from_name(nullptr, "ED25519", nullptr);
    if (!ctx)
    {
        return toolkit::make_error("failed to create context: {}", GetSSLErrorStack());
    }

    auto guard_ctx = toolkit::defer(EVP_PKEY_CTX_free, ctx);

    if (EVP_PKEY_fromdata_init(ctx) <= 0)
    {
        return toolkit::make_error("failed to initialize public key from data: {}", GetSSLErrorStack());
    }

    if (!q.empty() && q[0] == 0x40)
    {
        q = q.subspan(1);
    }

    if (q.size() != 0x20)
    {
        return toolkit::make_error("invalid key size: {}", q.size());
    }

    OSSL_PARAM params[]
    {
        OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, const_cast<char *>(group), strlen(group)),
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_END,
    };

    EVP_PKEY *public_key{};
    if (EVP_PKEY_fromdata(ctx, &public_key, EVP_PKEY_PUBLIC_KEY, params) <= 0)
    {
        return toolkit::make_error("failed to create public key from data: {}", GetSSLErrorStack());
    }

    return public_key;
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_DSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto p = *cursor++;
    auto q = *cursor++;
    auto g = *cursor++;
    auto y = *cursor++;

    auto *ctx = EVP_PKEY_CTX_new_from_name(nullptr, "DSA", nullptr);
    if (!ctx)
    {
        return toolkit::make_error("failed to create context: {}", GetSSLErrorStack());
    }

    auto guard_ctx = toolkit::defer(EVP_PKEY_CTX_free, ctx);

    if (EVP_PKEY_fromdata_init(ctx) <= 0)
    {
        return toolkit::make_error("failed to initialize public key from data: {}", GetSSLErrorStack());
    }

    OSSL_PARAM params[]
    {
        OSSL_PARAM_BN("p", const_cast<uint8_t *>(p.data()), p.size()),
        OSSL_PARAM_BN("q", const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_BN("g", const_cast<uint8_t *>(g.data()), g.size()),
        OSSL_PARAM_BN("y", const_cast<uint8_t *>(y.data()), y.size()),
        OSSL_PARAM_END,
    };

    EVP_PKEY *public_key{};
    if (EVP_PKEY_fromdata(ctx, &public_key, EVP_PKEY_PUBLIC_KEY, params) <= 0)
    {
        return toolkit::make_error("failed to create public key from data: {}", GetSSLErrorStack());
    }

    return public_key;
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey(const PublicKey &key)
{
    switch (key.Algorithm)
    {
    case PublicKeyAlgorithmID::RSA_ES:
    case PublicKeyAlgorithmID::RSA_EO:
    case PublicKeyAlgorithmID::RSA_SO:
        return CreateOpenSSLPublicKey_RSA(key.Material);

    case PublicKeyAlgorithmID::ECDSA:
    case PublicKeyAlgorithmID::ECDH:
        return CreateOpenSSLPublicKey_EC(key.Material);

    case PublicKeyAlgorithmID::EdDSA:
        return CreateOpenSSLPublicKey_EdDSA(key.Material);

    case PublicKeyAlgorithmID::DSA:
        return CreateOpenSSLPublicKey_DSA(key.Material);

    default:
        return toolkit::make_error("unsupported key algorithm {}.", key.Algorithm);
    }
}
