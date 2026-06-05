#include <unvm/pgp.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>
#include <toolkit/result.hxx>

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>

#include <cstring>
#include <iostream>

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

static toolkit::result<EVP_PKEY *> create_public_key(const char *name, OSSL_PARAM params[])
{
    auto *ctx = EVP_PKEY_CTX_new_from_name(nullptr, name, nullptr);
    if (!ctx)
    {
        return toolkit::make_error("failed to create context for name '{}': {}", name, unvm::GetSSLErrorStack());
    }

    auto guard_ctx = toolkit::defer(EVP_PKEY_CTX_free, ctx);

    // print_settable_params(ctx, name);

    if (EVP_PKEY_fromdata_init(ctx) <= 0)
    {
        return toolkit::make_error("failed to initialize public key from data: {}", unvm::GetSSLErrorStack());
    }

    EVP_PKEY *public_key{};
    if (EVP_PKEY_fromdata(ctx, &public_key, EVP_PKEY_PUBLIC_KEY, params) <= 0)
    {
        return toolkit::make_error("failed to create public key from data: {}", unvm::GetSSLErrorStack());
    }

    return public_key;
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_RSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto n = cursor.mpi();
    auto e = cursor.mpi();

    OSSL_PARAM params[]
    {
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_N, const_cast<uint8_t *>(n.data()), n.size()),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_E, const_cast<uint8_t *>(e.data()), e.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("RSA", params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_DSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto p = cursor.mpi();
    auto q = cursor.mpi();
    auto g = cursor.mpi();
    auto y = cursor.mpi();

    OSSL_PARAM params[]
    {
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_FFC_P, const_cast<uint8_t *>(p.data()), p.size()),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_FFC_Q, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_FFC_G, const_cast<uint8_t *>(g.data()), g.size()),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(y.data()), y.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("DSA", params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_Elgamal(std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto p = cursor.mpi();
    auto g = cursor.mpi();
    auto y = cursor.mpi();

    OSSL_PARAM params[]
    {
        OSSL_PARAM_BN("p", const_cast<uint8_t *>(p.data()), p.size()),
        OSSL_PARAM_BN("g", const_cast<uint8_t *>(g.data()), g.size()),
        OSSL_PARAM_BN("y", const_cast<uint8_t *>(y.data()), y.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("Elgamal", params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_ECDSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto curve = cursor.curve();
    auto q = cursor.mpi();

    if (!q.empty() && q[0] == 0x40)
    {
        q = q.subspan(1);
    }

    if (q.size() != 0x20)
    {
        return toolkit::make_error("invalid key size: {}", q.size());
    }

    auto *group = ToString(curve);

    OSSL_PARAM params[]
    {
        OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, const_cast<char *>(group), strlen(group)),
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("EC", params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_EdDSA(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto curve = cursor.curve();
    auto q = cursor.mpi();

    if (!q.empty() && q[0] == 0x40)
    {
        q = q.subspan(1);
    }

    if (q.size() != 0x20)
    {
        return toolkit::make_error("invalid key size: {}", q.size());
    }

    auto *group = ToString(curve);

    OSSL_PARAM params[]
    {
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_END,
    };

    return create_public_key(group, params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_ECDH(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto curve = cursor.curve();
    auto q = cursor.mpi();
    auto kdf = cursor.kdf();

    if (!q.empty() && q[0] == 0x40)
    {
        q = q.subspan(1);
    }

    if (q.size() != 0x20)
    {
        return toolkit::make_error("invalid key size: {}", q.size());
    }

    auto *group = ToString(curve);

    OSSL_PARAM params[]
    {
        OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, const_cast<char *>(group), strlen(group)),
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("EC", params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_X25519(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto q = cursor.bytes(32);

    OSSL_PARAM params[]
    {
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("X25519", params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_X448(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto q = cursor.bytes(56);

    OSSL_PARAM params[]
    {
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("X448", params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_Ed25519(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto q = cursor.bytes(32);

    OSSL_PARAM params[]
    {
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("ED25519", params);
}

toolkit::result<EVP_PKEY *> unvm::pgp::CreateOpenSSLPublicKey_Ed448(const std::span<const uint8_t> material)
{
    MPIIterator cursor(material.data());

    auto q = cursor.bytes(57);

    OSSL_PARAM params[]
    {
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, const_cast<uint8_t *>(q.data()), q.size()),
        OSSL_PARAM_END,
    };

    return create_public_key("ED448", params);
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
    case PublicKeyAlgorithmID::Elgamal_ES:
        return CreateOpenSSLPublicKey_Elgamal(key.Material);
    
    case PublicKeyAlgorithmID::ECDSA:
        return CreateOpenSSLPublicKey_ECDSA(key.Material);

    case PublicKeyAlgorithmID::EdDSA:
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
