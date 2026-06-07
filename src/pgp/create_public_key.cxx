#include <unvm/pgp.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>
#include <toolkit/result.hxx>

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/param_build.h>

#include <cstring>
#include <iostream>

class ParamBuilder
{
public:
    ParamBuilder()
    {
        bld = OSSL_PARAM_BLD_new();
    }

    ~ParamBuilder()
    {
        OSSL_PARAM_BLD_free(bld);
        OSSL_PARAM_free(param);

        for (auto *bn : bns)
        {
            BN_free(bn);
        }
    }

    void PushBN(const char *key, const std::span<const uint8_t> buffer)
    {
        auto *bn = BN_new();
        BN_bin2bn(buffer.data(), static_cast<int>(buffer.size()), bn);

        bns.push_back(bn);
        OSSL_PARAM_BLD_push_BN(bld, key, bn);
    }

    void PushUTF8String(const char *key, const char *str, size_t len = 0)
    {
        if (!len)
        {
            len = strlen(str);
        }

        OSSL_PARAM_BLD_push_utf8_string(bld, key, str, len);
    }

    void PushOctetString(const char *key, const std::span<const uint8_t> buffer)
    {
        OSSL_PARAM_BLD_push_octet_string(bld, key, buffer.data(), buffer.size());
    }

    OSSL_PARAM *Build()
    {
        return param = OSSL_PARAM_BLD_to_param(bld);
    }

private:
    OSSL_PARAM_BLD *bld;
    OSSL_PARAM *param{};

    std::vector<BIGNUM *> bns;
};

static void print_settable_params(EVP_PKEY_CTX *ctx, const char *name)
{
    auto settable = EVP_PKEY_fromdata_settable(ctx, EVP_PKEY_PUBLIC_KEY);
    if (!settable)
    {
        std::cerr << "failed to get settable params for name '" << name << "'." << std::endl;
        return;
    }

    std::cerr << "settable params for name '" << name << "':" << std::endl;

    for (; settable->key; ++settable)
    {
        const char *data_type;

        switch (settable->data_type)
        {
        case OSSL_PARAM_INTEGER:
            data_type = "INTEGER";
            break;
        case OSSL_PARAM_UNSIGNED_INTEGER:
            data_type = "UNSIGNED_INTEGER";
            break;
        case OSSL_PARAM_REAL:
            data_type = "REAL";
            break;
        case OSSL_PARAM_UTF8_STRING:
            data_type = "UTF8_STRING";
            break;
        case OSSL_PARAM_OCTET_STRING:
            data_type = "OCTET_STRING";
            break;
        case OSSL_PARAM_UTF8_PTR:
            data_type = "UTF8_PTR";
            break;
        case OSSL_PARAM_OCTET_PTR:
            data_type = "OCTET_PTR";
            break;
        default:
            data_type = nullptr;
            break;
        }

        std::cerr << settable->key << ": " << data_type << " ";
        if (settable->data_size)
        {
            std::cerr << "(size: " << settable->data_size << ")" << std::endl;
        }
        else
        {
            std::cerr << "(no size limit)" << std::endl;
        }
    }
}

static toolkit::result<EVP_PKEY *> create_public_key(const char *name, OSSL_PARAM param[])
{
    auto *ctx = EVP_PKEY_CTX_new_from_name(nullptr, name, nullptr);
    if (!ctx)
    {
        return toolkit::make_error("failed to create context for name '{}': {}", name, unvm::GetSSLErrorStack());
    }

    auto guard_ctx = toolkit::defer(EVP_PKEY_CTX_free, ctx);

    // print_settable_params(ctx, name);
    (void) print_settable_params;

    if (EVP_PKEY_fromdata_init(ctx) <= 0)
    {
        return toolkit::make_error("failed to initialize public key from data: {}", unvm::GetSSLErrorStack());
    }

    EVP_PKEY *public_key{};
    if (EVP_PKEY_fromdata(ctx, &public_key, EVP_PKEY_PUBLIC_KEY, param) <= 0)
    {
        return toolkit::make_error("failed to create public key from data: {}", unvm::GetSSLErrorStack());
    }

    return public_key;
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_RSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    std::span<const uint8_t> n, e;

    if (auto res = cursor.mpi() >> n; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> e; !res)
    {
        return res;
    }

    ParamBuilder pb;
    pb.PushBN(OSSL_PKEY_PARAM_RSA_N, n);
    pb.PushBN(OSSL_PKEY_PARAM_RSA_E, e);

    return create_public_key("RSA", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_DSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    std::span<const uint8_t> p, q, g, y;

    if (auto res = cursor.mpi() >> p; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> q; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> g; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> y; !res)
    {
        return res;
    }

    ParamBuilder pb;
    pb.PushBN(OSSL_PKEY_PARAM_FFC_P, p);
    pb.PushBN(OSSL_PKEY_PARAM_FFC_Q, q);
    pb.PushBN(OSSL_PKEY_PARAM_FFC_G, g);
    pb.PushBN(OSSL_PKEY_PARAM_PUB_KEY, y);

    return create_public_key("DSA", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_Elgamal(std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    std::span<const uint8_t> p, g, y;

    if (auto res = cursor.mpi() >> p; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> g; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> y; !res)
    {
        return res;
    }

    ParamBuilder pb;
    pb.PushBN("p", p);
    pb.PushBN("g", g);
    pb.PushBN("y", y);

    return create_public_key("Elgamal", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_ECDSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    CurveOID curve;
    std::span<const uint8_t> q;

    if (auto res = cursor.curve() >> curve; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> q; !res)
    {
        return res;
    }

    if (!q.empty() && q[0] == 0x40)
    {
        q = q.subspan(1);
    }

    if (q.size() != 0x20)
    {
        return toolkit::make_error("invalid key size: {}", q.size());
    }

    auto *group = ToString(curve);

    ParamBuilder pb;
    pb.PushUTF8String(OSSL_PKEY_PARAM_GROUP_NAME, group);
    pb.PushOctetString(OSSL_PKEY_PARAM_PUB_KEY, q);

    return create_public_key("EC", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_EdDSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    CurveOID curve;
    std::span<const uint8_t> q;

    if (auto res = cursor.curve() >> curve; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> q; !res)
    {
        return res;
    }

    if (!q.empty() && q[0] == 0x40)
    {
        q = q.subspan(1);
    }

    if (q.size() != 0x20)
    {
        return toolkit::make_error("invalid key size: {}", q.size());
    }

    auto *group = ToString(curve);

    ParamBuilder pb;
    pb.PushOctetString(OSSL_PKEY_PARAM_PUB_KEY, q);

    return create_public_key(group, pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_ECDH(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    CurveOID curve;
    std::span<const uint8_t> q;
    KDF kdf{};

    if (auto res = cursor.curve() >> curve; !res)
    {
        return res;
    }

    if (auto res = cursor.mpi() >> q; !res)
    {
        return res;
    }

    if (auto res = cursor.kdf() >> kdf; !res)
    {
        return res;
    }

    (void) kdf;

    if (!q.empty() && q[0] == 0x40)
    {
        q = q.subspan(1);
    }

    if (q.size() != 0x20)
    {
        return toolkit::make_error("invalid key size: {}", q.size());
    }

    auto *group = ToString(curve);

    ParamBuilder pb;
    pb.PushUTF8String(OSSL_PKEY_PARAM_GROUP_NAME, group);
    pb.PushOctetString(OSSL_PKEY_PARAM_PUB_KEY, q);

    return create_public_key("EC", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_X25519(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    std::span<const uint8_t> q;

    if (auto res = cursor.bytes(32) >> q; !res)
    {
        return res;
    }

    ParamBuilder pb;
    pb.PushOctetString(OSSL_PKEY_PARAM_PUB_KEY, q);

    return create_public_key("X25519", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_X448(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    std::span<const uint8_t> q;

    if (auto res = cursor.bytes(56) >> q; !res)
    {
        return res;
    }

    ParamBuilder pb;
    pb.PushOctetString(OSSL_PKEY_PARAM_PUB_KEY, q);

    return create_public_key("X448", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_Ed25519(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    std::span<const uint8_t> q;

    if (auto res = cursor.bytes(32) >> q; !res)
    {
        return res;
    }

    ParamBuilder pb;
    pb.PushOctetString(OSSL_PKEY_PARAM_PUB_KEY, q);

    return create_public_key("ED25519", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_Ed448(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material);

    std::span<const uint8_t> q;

    if (auto res = cursor.bytes(57) >> q; !res)
    {
        return res;
    }

    ParamBuilder pb;
    pb.PushOctetString(OSSL_PKEY_PARAM_PUB_KEY, q);

    return create_public_key("ED448", pb.Build());
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey(const PublicKey &key)
{
    switch (key.Algorithm)
    {
    case PublicKeyAlgorithmID::RSA_ES:
    case PublicKeyAlgorithmID::RSA_EO:
    case PublicKeyAlgorithmID::RSA_SO:
        return CreateOpenSSLPublicKey_RSA(key.Material);

    case PublicKeyAlgorithmID::DSA:
        return CreateOpenSSLPublicKey_DSA(key.Material);

    case PublicKeyAlgorithmID::Elgamal_EO:
        return CreateOpenSSLPublicKey_Elgamal(key.Material);

    case PublicKeyAlgorithmID::ECDSA:
        return CreateOpenSSLPublicKey_ECDSA(key.Material);

    case PublicKeyAlgorithmID::EdDSALegacy:
        return CreateOpenSSLPublicKey_EdDSA(key.Material);

    case PublicKeyAlgorithmID::ECDH:
        return CreateOpenSSLPublicKey_ECDH(key.Material);

    case PublicKeyAlgorithmID::X25519:
        return CreateOpenSSLPublicKey_X25519(key.Material);

    case PublicKeyAlgorithmID::X448:
        return CreateOpenSSLPublicKey_X448(key.Material);

    case PublicKeyAlgorithmID::Ed25519:
        return CreateOpenSSLPublicKey_Ed25519(key.Material);

    case PublicKeyAlgorithmID::Ed448:
        return CreateOpenSSLPublicKey_Ed448(key.Material);

    default:
        return toolkit::make_error("unsupported key algorithm {}.", key.Algorithm);
    }
}
