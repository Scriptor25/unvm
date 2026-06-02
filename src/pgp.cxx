#include <unvm/pgp.hxx>
#include <unvm/util.hxx>

#include <toolkit/defer.hxx>
#include <toolkit/result.hxx>

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>

#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>

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

std::string unvm::pgp::ToString(const PacketTypeID packet_type)
{
    static const std::unordered_map<PacketTypeID, const char *> map
    {
        { PacketTypeID::PublicKeyEncryptedSessionKeyPacket, "PublicKeyEncryptedSessionKeyPacket" },
        { PacketTypeID::SignaturePacket, "SignaturePacket" },
        { PacketTypeID::SymmetricKeyEncryptedSessionKeyPacket, "SymmetricKeyEncryptedSessionKeyPacket" },
        { PacketTypeID::OnePassSignaturePacket, "OnePassSignaturePacket" },
        { PacketTypeID::SecretKeyPacket, "SecretKeyPacket" },
        { PacketTypeID::PublicKeyPacket, "PublicKeyPacket" },
        { PacketTypeID::SecretSubkeyPacket, "SecretSubkeyPacket" },
        { PacketTypeID::CompressedDataPacket, "CompressedDataPacket" },
        { PacketTypeID::SymmetricallyEncryptedDataPacket, "SymmetricallyEncryptedDataPacket" },
        { PacketTypeID::MarkerPacket, "MarkerPacket" },
        { PacketTypeID::LiteralDataPacket, "LiteralDataPacket" },
        { PacketTypeID::TrustPacket, "TrustPacket" },
        { PacketTypeID::UserIDPacket, "UserIDPacket" },
        { PacketTypeID::PublicSubkeyPacket, "PublicSubkeyPacket" },
        { PacketTypeID::UserAttributePacket, "UserAttributePacket" },
        { PacketTypeID::SymEncryptedAndIntegrityProtectedDataPacket, "SymEncryptedAndIntegrityProtectedDataPacket" },
        { PacketTypeID::ModificationDetectionCodePacket, "ModificationDetectionCodePacket" },
    };

    if (const auto it = map.find(packet_type); it != map.end())
    {
        return it->second;
    }

    return ToHexString(static_cast<uint8_t>(packet_type));
}

std::string unvm::pgp::ToString(const SubpacketTypeID subpacket_type)
{
    static const std::unordered_map<SubpacketTypeID, const char *> map
    {
        { SubpacketTypeID::SignatureCreationTime, "SignatureCreationTime" },
        { SubpacketTypeID::SignatureExpirationTime, "SignatureExpirationTime" },
        { SubpacketTypeID::ExportableCertification, "ExportableCertification" },
        { SubpacketTypeID::TrustSignature, "TrustSignature" },
        { SubpacketTypeID::RegularExpression, "RegularExpression" },
        { SubpacketTypeID::Revocable, "Revocable" },
        { SubpacketTypeID::KeyExpirationTime, "KeyExpirationTime" },
        { SubpacketTypeID::PreferredSymmetricAlgorithms, "PreferredSymmetricAlgorithms" },
        { SubpacketTypeID::RevocationKey, "RevocationKey" },
        { SubpacketTypeID::IssuerKeyID, "IssuerKeyID" },
        { SubpacketTypeID::NotationData, "NotationData" },
        { SubpacketTypeID::PreferredHashAlgorithms, "PreferredHashAlgorithms" },
        { SubpacketTypeID::PreferredCompressionAlgorithms, "PreferredCompressionAlgorithms" },
        { SubpacketTypeID::KeyServerPreferences, "KeyServerPreferences" },
        { SubpacketTypeID::PreferredKeyServer, "PreferredKeyServer" },
        { SubpacketTypeID::PrimaryUserID, "PrimaryUserID" },
        { SubpacketTypeID::PolicyURI, "PolicyURI" },
        { SubpacketTypeID::KeyFlags, "KeyFlags" },
        { SubpacketTypeID::SignersUserID, "SignersUserID" },
        { SubpacketTypeID::ReasonForRevocation, "ReasonForRevocation" },
        { SubpacketTypeID::Features, "Features" },
        { SubpacketTypeID::SignatureTarget, "SignatureTarget" },
        { SubpacketTypeID::EmbeddedSignature, "EmbeddedSignature" },
        { SubpacketTypeID::IssuerFingerprint, "IssuerFingerprint" },
        { SubpacketTypeID::IntendedRecipientFingerprint, "IntendedRecipientFingerprint" },
        { SubpacketTypeID::PreferredAEADCipherSuites, "PreferredAEADCipherSuites" },
    };

    if (const auto it = map.find(subpacket_type); it != map.end())
    {
        return it->second;
    }

    return ToHexString(static_cast<uint8_t>(subpacket_type));
}

std::string unvm::pgp::ToString(const SignatureTypeID signature_type)
{
    static const std::unordered_map<SignatureTypeID, const char *> map
    {
        { SignatureTypeID::Binary, "Binary" },
        { SignatureTypeID::Text, "Text" },
        { SignatureTypeID::Standalone, "Standalone" },
        { SignatureTypeID::GenericCertification, "GenericCertification" },
        { SignatureTypeID::PersonaCertification, "PersonaCertification" },
        { SignatureTypeID::CasualCertification, "CasualCertification" },
        { SignatureTypeID::PositiveCertification, "PositiveCertification" },
        { SignatureTypeID::SubkeyBinding, "SubkeyBinding" },
        { SignatureTypeID::PrimaryKeyBinding, "PrimaryKeyBinding" },
        { SignatureTypeID::DirectKey, "DirectKey" },
        { SignatureTypeID::KeyRevocation, "KeyRevocation" },
        { SignatureTypeID::SubkeyRevocation, "SubkeyRevocation" },
        { SignatureTypeID::CertificationRevocation, "CertificationRevocation" },
        { SignatureTypeID::Timestamp, "Timestamp" },
        { SignatureTypeID::ThirdPartyConfirmation, "ThirdPartyConfirmation" },
    };

    if (const auto it = map.find(signature_type); it != map.end())
    {
        return it->second;
    }

    return ToHexString(static_cast<uint8_t>(signature_type));
}

std::string unvm::pgp::ToString(const PublicKeyAlgorithmID algorithm)
{
    static const std::unordered_map<PublicKeyAlgorithmID, const char *> map
    {
        { PublicKeyAlgorithmID::RSA_ES, "RSA (Encrypt and Signature)" },
        { PublicKeyAlgorithmID::RSA_EO, "RSA (Encrypt Only)" },
        { PublicKeyAlgorithmID::RSA_SO, "RSA (Signature Only)" },
        { PublicKeyAlgorithmID::Elgamal_EO, "Elgamal (Encrypt Only)" },
        { PublicKeyAlgorithmID::DSA, "DSA" },
        { PublicKeyAlgorithmID::ECDH, "ECDH" },
        { PublicKeyAlgorithmID::ECDSA, "ECDSA" },
        { PublicKeyAlgorithmID::Elgamal_ES, "Elgamal (Encrypt and Signature)" },
        { PublicKeyAlgorithmID::DiffieHellman, "Diffie-Hellman" },
        { PublicKeyAlgorithmID::EdDSA, "EdDSA" },
        { PublicKeyAlgorithmID::AEDH, "AEDH" },
        { PublicKeyAlgorithmID::AEDSA, "AEDSA" },
        { PublicKeyAlgorithmID::X25519, "X25519" },
        { PublicKeyAlgorithmID::X448, "X448" },
        { PublicKeyAlgorithmID::Ed25519, "Ed25519" },
        { PublicKeyAlgorithmID::Ed448, "Ed448" },
    };

    if (const auto it = map.find(algorithm); it != map.end())
    {
        return it->second;
    }

    return ToHexString(static_cast<uint8_t>(algorithm));
}

std::string unvm::pgp::ToString(const HashAlgorithmID algorithm)
{
    static const std::unordered_map<HashAlgorithmID, const char *> map
    {
        { HashAlgorithmID::MD5, "MD5" },
        { HashAlgorithmID::SHA1, "SHA1" },
        { HashAlgorithmID::RIPEMD160, "RIPEMD160" },
        { HashAlgorithmID::SHA256, "SHA256" },
        { HashAlgorithmID::SHA384, "SHA384" },
        { HashAlgorithmID::SHA512, "SHA512" },
        { HashAlgorithmID::SHA224, "SHA224" },
        { HashAlgorithmID::SHA3_256, "SHA3_256" },
        { HashAlgorithmID::SHA3_512, "SHA3_512" },
    };

    if (const auto it = map.find(algorithm); it != map.end())
    {
        return it->second;
    }

    return ToHexString(static_cast<uint8_t>(algorithm));
}

std::string unvm::pgp::ToString(CompressionAlgorithmID algorithm)
{
    static const std::unordered_map<CompressionAlgorithmID, const char *> map
    {
        { CompressionAlgorithmID::Uncompressed, "uncompressed" },
        { CompressionAlgorithmID::ZIP, "zip" },
        { CompressionAlgorithmID::ZLIB, "zlib" },
        { CompressionAlgorithmID::BZIP2, "bzip2" },
    };

    if (const auto it = map.find(algorithm); it != map.end())
    {
        return it->second;
    }

    return ToHexString(static_cast<uint8_t>(algorithm));
}

std::string unvm::pgp::ToString(SymmetricAlgorithmID algorithm)
{
    static const std::unordered_map<SymmetricAlgorithmID, const char *> map
    {
        { SymmetricAlgorithmID::Plain, "plain" },
        { SymmetricAlgorithmID::IDEA, "idea" },
        { SymmetricAlgorithmID::TripleDES, "tripledes" },
        { SymmetricAlgorithmID::CAST5, "cast5" },
        { SymmetricAlgorithmID::Blowfish, "blowfish" },
        { SymmetricAlgorithmID::AES_128, "aes_128" },
        { SymmetricAlgorithmID::AES_192, "aes_192" },
        { SymmetricAlgorithmID::AES_256, "aes_256" },
        { SymmetricAlgorithmID::Twofish, "twofish" },
        { SymmetricAlgorithmID::Camellia_128, "camellia_128" },
        { SymmetricAlgorithmID::Camellia_192, "camellia_192" },
        { SymmetricAlgorithmID::Camellia_256, "camellia_256" },
    };

    if (const auto it = map.find(algorithm); it != map.end())
    {
        return it->second;
    }

    return ToHexString(static_cast<uint8_t>(algorithm));
}

const char *unvm::pgp::ToString(const CurveOID curve)
{
    static const std::unordered_map<CurveOID, const char *> map
    {
        { CurveOID::NIST_P256, "prime256v1" },
        { CurveOID::NIST_P384, "secp384r1" },
        { CurveOID::NIST_P521, "secp521r1" },
        { CurveOID::Brainpool_P256r1, "brainpoolP256r1" },
        { CurveOID::Brainpool_P384r1, "brainpoolP384r1" },
        { CurveOID::Brainpool_P512r1, "brainpoolP512r1" },
        { CurveOID::Ed25519, "ED25519" },
        { CurveOID::Curve25519, "X25519" },
    };

    if (const auto it = map.find(curve); it != map.end())
    {
        return it->second;
    }

    return nullptr;
}

std::string unvm::pgp::ToHexString(const uint8_t value)
{
    char buffer[2];

    buffer[0] = static_cast<char>(value >> 4 & 0xF);
    buffer[1] = static_cast<char>(value & 0xF);

    buffer[0] = static_cast<char>(buffer[0] + (buffer[0] < 10 ? '0' : 'A' - 10));
    buffer[1] = static_cast<char>(buffer[1] + (buffer[1] < 10 ? '0' : 'A' - 10));

    return { buffer, sizeof(buffer) };
}

std::string unvm::pgp::ToHexString(const std::span<const uint8_t> buffer)
{
    std::stringstream stream;

    for (const auto octet : buffer)
    {
        stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned>(octet);
    }

    return stream.str();
}

void unvm::pgp::PacketHeader::Parse(
    PacketTypeID &packet_type,
    uint32_t &packet_length,
    uint8_t &header_length,
    bool &partial) const
{
    if (Tag.NewFormat)
    {
        auto *tag = &Tag;
        packet_type = tag->PacketType;

        if (const auto fst = Length[0]; fst < 0xC0)
        {
            header_length = 2;
            packet_length = fst;
            partial = false;
        }
        else if (fst < 0xE0)
        {
            const auto snd = Length[1];

            header_length = 3;
            packet_length = ((fst - 0xC0) << 8) + snd + 0xC0;
            partial = false;
        }
        else if (fst < 0xFF)
        {
            header_length = 2;
            packet_length = 1 << (fst & 0x1F);
            partial = true;
        }
        else // if (fst == 0xFF)
        {
            header_length = 6;
            packet_length = scalar<4>(&Length[1]);
            partial = false;
        }
    }
    else
    {
        auto *tag = reinterpret_cast<const PacketTagLegacy *>(&Tag);
        packet_type = tag->PacketType;

        const auto length_bytes = 1 << tag->LengthType;
        header_length = 1 + length_bytes;

        switch (tag->LengthType)
        {
        case 0:
            packet_length = Length[0];
            partial = false;
            break;
        case 1:
            packet_length = scalar<2>(Length);
            partial = false;
            break;
        case 2:
            packet_length = scalar<4>(Length);
            partial = false;
            break;
        case 3:
        default:
            header_length = 1;
            packet_length = 0;
            partial = false;
            break;
        }
    }
}

unvm::pgp::SubpacketDescriptor unvm::pgp::DescribeSubpacket(const uint8_t *subpacket)
{
    uint32_t subpacket_length;
    uint8_t length_length;

    if (const auto fst = subpacket[0]; fst < 0xC0)
    {
        subpacket_length = fst;
        length_length = 1;
    }
    else if (fst < 0xFF)
    {
        const auto snd = subpacket[1];

        subpacket_length = ((fst - 0xC0) << 8) + snd + 0xC0;
        length_length = 2;
    }
    else // if (fst == 0xFF)
    {
        subpacket_length = scalar<4>(&subpacket[1]);
        length_length = 5;
    }

    auto *next = subpacket + length_length + subpacket_length;

    return {
        .Subpacket = subpacket,
        .Next = next,
        .Length = subpacket_length,
        .Data = reinterpret_cast<const SubpacketData *>(subpacket + length_length),
    };
}

bool unvm::pgp::SubpacketIterator::operator!=(const SubpacketIterator &it) const
{
    return ptr != it.ptr;
}

unvm::pgp::SubpacketDescriptor unvm::pgp::SubpacketIterator::operator*() const
{
    return DescribeSubpacket(ptr);
}

unvm::pgp::SubpacketIterator &unvm::pgp::SubpacketIterator::operator++()
{
    const auto descriptor = DescribeSubpacket(ptr);

    ptr = descriptor.Next;

    return *this;
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterator::operator++(int)
{
    const auto descriptor = DescribeSubpacket(ptr);

    auto *pre = ptr;

    ptr = descriptor.Next;

    return { pre };
}

unvm::pgp::SubpacketIterable::SubpacketIterable(const uint8_t *first, const uint8_t *last)
    : block(first, last)
{
}

unvm::pgp::SubpacketIterable::SubpacketIterable(const std::span<const uint8_t> block)
    : block(block)
{
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterable::begin() const
{
    return { block.data() };
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterable::end() const
{
    return { block.data() + block.size() };
}

size_t unvm::pgp::SubpacketIterable::size() const
{
    return block.size();
}

bool unvm::pgp::MPIIterator::operator!=(const MPIIterator &it) const
{
    return ptr != it.ptr;
}

std::span<const uint8_t> unvm::pgp::MPIIterator::operator*() const
{
    const auto bit_count = scalar<2>(ptr);
    auto byte_count = (bit_count + 7u) / 8u;

    return { ptr + 2, byte_count };
}

unvm::pgp::MPIIterator &unvm::pgp::MPIIterator::operator++()
{
    const auto bit_count = scalar<2>(ptr);
    const auto byte_count = (bit_count + 7u) / 8u;

    ptr += 2 + byte_count;

    return *this;
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterator::operator++(int)
{
    const auto bit_count = scalar<2>(ptr);
    const auto byte_count = (bit_count + 7u) / 8u;

    auto *pre = ptr;

    ptr += 2 + byte_count;

    return { pre };
}

unvm::pgp::CurveOID unvm::pgp::MPIIterator::curve()
{
    const auto len = *ptr;
    const std::span oid(ptr + 1, len);

    ptr += 1 + len;

    if (oid == OID_NIST_P256)
    {
        return CurveOID::NIST_P256;
    }

    if (oid == OID_NIST_P384)
    {
        return CurveOID::NIST_P384;
    }

    if (oid == OID_NIST_P521)
    {
        return CurveOID::NIST_P521;
    }

    if (oid == OID_Brainpool_P256r1)
    {
        return CurveOID::Brainpool_P256r1;
    }

    if (oid == OID_Brainpool_P384r1)
    {
        return CurveOID::Brainpool_P384r1;
    }

    if (oid == OID_Brainpool_P512r1)
    {
        return CurveOID::Brainpool_P512r1;
    }

    if (oid == OID_Ed25519)
    {
        return CurveOID::Ed25519;
    }

    if (oid == OID_Curve25519)
    {
        return CurveOID::Curve25519;
    }

    return CurveOID::Error;
}

unvm::pgp::MPIIterable::MPIIterable(const uint8_t *first, const uint8_t *last)
    : block(first, last)
{
}

unvm::pgp::MPIIterable::MPIIterable(const std::span<const uint8_t> block)
    : block(block)
{
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterable::begin() const
{
    return { block.data() };
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterable::end() const
{
    return { block.data() + block.size() };
}

uint16_t unvm::pgp::SubpacketSetV4::GetLength() const
{
    return scalar(Length);
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV4::begin() const
{
    return { Data };
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV4::end() const
{
    return { Data + GetLength() };
}

uint32_t unvm::pgp::SubpacketSetV6::GetLength() const
{
    return scalar(Length);
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV6::begin() const
{
    return { Data };
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV6::end() const
{
    return { Data + GetLength() };
}

[[nodiscard]] static toolkit::result<> evaluate_subpacket(
    const unvm::pgp::SubpacketData *data,
    const uint32_t length,
    unvm::pgp::Signature &signature)
{
    switch (data->Type)
    {
    case unvm::pgp::SubpacketTypeID::KeyFlags:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::KeyFlagsSubpacket *>(data);

        const std::span flags(packet->Flags, length - 1);
        for (size_t i = 0; i < flags.size(); ++i)
        {
            signature.KeyFlags |= flags[i] << (i * 8);
        }

        break;
    }
    case unvm::pgp::SubpacketTypeID::PrimaryUserID:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::PrimaryUserIDSubpacket *>(data);

        signature.PrimaryUserID = packet->Primary == 0x01;

        break;
    }
    case unvm::pgp::SubpacketTypeID::PreferredSymmetricAlgorithms:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::PreferredSymmetricAlgorithmsSubpacket *>(data);

        signature.PreferredSymmetricAlgorithms = { packet->Algorithms, length - 1 };

        break;
    }
    case unvm::pgp::SubpacketTypeID::PreferredHashAlgorithms:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::PreferredHashAlgorithmsSubpacket *>(data);

        signature.PreferredHashAlgorithms = { packet->Algorithms, length - 1 };

        break;
    }
    case unvm::pgp::SubpacketTypeID::PreferredCompressionAlgorithms:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::PreferredCompressionAlgorithmsSubpacket *>(data);

        signature.PreferredCompressionAlgorithms = { packet->Algorithms, length - 1 };

        break;
    }
    case unvm::pgp::SubpacketTypeID::IssuerFingerprint:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::IssuerFingerprintSubpacket *>(data);

        std::span<const uint8_t> fingerprint;
        std::span<const uint8_t> keyid;

        switch (packet->KeyVersion)
        {
        case 0x04:
            fingerprint = { packet->Fingerprint, packet->Fingerprint + 20 };
            keyid = fingerprint.subspan(12, 8);
            break;
        case 0x06:
            fingerprint = { packet->Fingerprint, packet->Fingerprint + 32 };
            keyid = fingerprint.subspan(0, 8);
            break;
        default:
            return toolkit::make_error("unsupported key version {:02x}", packet->KeyVersion);
        }

        signature.IssuerFingerprint = fingerprint;
        signature.IssuerKeyID = keyid;

        break;
    }
    case unvm::pgp::SubpacketTypeID::IssuerKeyID:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::IssuerKeyIDSubpacket *>(data);

        signature.IssuerKeyID = packet->KeyID;

        break;
    }
    case unvm::pgp::SubpacketTypeID::SignatureCreationTime:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::SignatureCreationTimeSubpacket *>(data);

        signature.SignatureCreationTime = unvm::pgp::scalar(packet->Time);

        break;
    }
    case unvm::pgp::SubpacketTypeID::Features:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::FeaturesSubpacket *>(data);

        const std::span flags(packet->Flags, length - 1);
        for (size_t i = 0; i < flags.size(); ++i)
        {
            signature.Features |= flags[i] << (i * 8);
        }

        break;
    }
    case unvm::pgp::SubpacketTypeID::KeyServerPreferences:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::KeyServerPreferencesSubpacket *>(data);

        const std::span flags(packet->Flags, length - 1);
        for (size_t i = 0; i < flags.size(); ++i)
        {
            signature.KeyServerPreferences |= flags[i] << (i * 8);
        }

        break;
    }
    case unvm::pgp::SubpacketTypeID::KeyExpirationTime:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::KeyExpirationTimeSubpacket *>(data);

        signature.KeyExpirationTime = unvm::pgp::scalar(packet->Time);

        break;
    }
    case unvm::pgp::SubpacketTypeID::EmbeddedSignature:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::EmbeddedSignatureSubpacket *>(data);

        unvm::pgp::Signature signature;
        if (auto res = unvm::pgp::ParseSignature(&packet->Packet, length - 1) >> signature; !res)
        {
            return res;
        }

        (void) signature;

        break;
    }
    case unvm::pgp::SubpacketTypeID::ReasonForRevocation:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::ReasonForRevocationSubpacket *>(data);

        signature.ReasonForRevocationCode = packet->Code;
        signature.ReasonForRevocationMessage = { reinterpret_cast<const char *>(packet->Reason), length - 2 };

        break;
    }
    default:
        break;
    }

    return {};
}

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

toolkit::result<unvm::pgp::Keyring> unvm::pgp::ParseKeyring(const std::span<const uint8_t> buffer)
{
    Keyring keyring;

    Certificate *current_certificate{};
    User *current_user{};
    Subkey *current_subkey{};

    auto *buffer_next = buffer.data();
    auto *buffer_end = buffer.data() + buffer.size();

    while (buffer_next != buffer_end)
    {
        auto *header = reinterpret_cast<const PacketHeader *>(buffer_next);

        PacketTypeID packet_type;
        uint32_t packet_length;
        uint8_t header_length;
        bool partial;

        header->Parse(packet_type, packet_length, header_length, partial);

        if (partial)
        {
            return toolkit::make_error("partial packets are not supported.");
        }

        auto *pointer = reinterpret_cast<const uint8_t *>(header) + header_length;

        switch (packet_type)
        {
        // begin certificate
        case PacketTypeID::PublicKeyPacket:
        {
            auto &certificate = keyring.emplace_back();

            current_certificate = &certificate;
            current_user = {};
            current_subkey = {};

            auto *packet = reinterpret_cast<const PublicKeyPacket *>(pointer);
            
            if (auto res = ParsePublicKey(packet, packet_length) >> certificate.Key; !res)
            {
                return res;
            }

            break;
        }
        // record user into certificate
        case PacketTypeID::UserIDPacket:
        {
            if (!current_certificate)
            {
                return toolkit::make_error("illegal user id packet without certificate.");
            }

            auto &user = current_certificate->Users.emplace_back();

            current_user = &user;
            current_subkey = {};

            user = {
                .Id = std::string(pointer, pointer + packet_length),
            };

            break;
        }
        // record subkey into certificate
        case PacketTypeID::PublicSubkeyPacket:
        {
            if (!current_certificate)
            {
                return toolkit::make_error("illegal subkey packet without certificate.");
            }

            auto &subkey = current_certificate->Subkeys.emplace_back();

            current_user = {};
            current_subkey = &subkey;

            auto *packet = reinterpret_cast<const PublicKeyPacket *>(pointer);

            if (auto res = ParsePublicKey(packet, packet_length) >> subkey.Key; !res)
            {
                return res;
            }

            break;
        }
        // record signature into user or subkey
        case PacketTypeID::SignaturePacket:
        {
            auto *packet = reinterpret_cast<const SignaturePacket *>(pointer);

            std::vector<Signature> *target;
            if (current_user && !current_subkey)
            {
                target = &current_user->Signatures;
            }
            else if (!current_user && current_subkey)
            {
                target = &current_subkey->Signatures;
            }
            else
            {
                return toolkit::make_error("illegal signature packet without user or subkey.");
            }

            auto &entry = target->emplace_back();

            if (auto res = ParseSignature(packet, packet_length) >> entry; !res)
            {
                return res;
            }

            break;
        }
        default:
            return toolkit::make_error("unsupported packet type {}", packet_type);
        }

        buffer_next += header_length + packet_length;
    }

    return keyring;
}

toolkit::result<unvm::pgp::PublicKey> unvm::pgp::ParsePublicKey(const PublicKeyPacket *packet, uint32_t packet_length)
{
    PublicKey key;

    auto *pointer = (const uint8_t *) packet;
    std::span data(pointer, packet_length);

    switch (packet->Version)
    {
    case 0x04:
    {
        auto *pkg = reinterpret_cast<const PublicKeyPacketV4 *>(packet);

        key.CreationTime = scalar(pkg->CreationTime);
        key.Algorithm = pkg->PublicKeyAlgorithm;
        key.Material = { pkg->Key, pointer + packet_length };

        const uint8_t magic = 0x99;

        if (auto res = digest(EVP_sha1(), magic, uint16_t(packet_length), data) >> key.Fingerprint; !res)
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

        const uint8_t magic = 0x9B;

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

toolkit::result<unvm::pgp::Signature> unvm::pgp::ParseSignature(const SignaturePacket *packet, uint32_t packet_length)
{
    auto *ptr = reinterpret_cast<const uint8_t *>(packet);

    std::span<const uint8_t> hashed_block;
    std::span<const uint8_t> unhashed_block;

    std::span<const uint8_t> hash_left_16_bit;
    std::span<const uint8_t> salt_material;

    std::span<const uint8_t> signature_material;

    switch (packet->Version)
    {
    case 0x04:
    {
        auto *hashed = reinterpret_cast<const SubpacketSetV4 *>(packet + 1);
        auto hashed_length = hashed->GetLength();

        hashed_block = { hashed->Data, hashed_length };

        auto *unhashed = reinterpret_cast<const SubpacketSetV4 *>(
            reinterpret_cast<const uint8_t *>(hashed) + 2 + hashed_length);
        auto unhashed_length = unhashed->GetLength();

        unhashed_block = { unhashed->Data, unhashed_length };

        auto *end = reinterpret_cast<const SignaturePacketEndV4 *>(
            reinterpret_cast<const uint8_t *>(unhashed) + 2 + unhashed_length);

        hash_left_16_bit = { end->HashLeft16Bit, 2 };

        auto *signature_block = reinterpret_cast<const SignatureBlock *>(end + 1);

        signature_material = { signature_block->Signature, ptr + packet_length };

        break;
    }
    case 0x06:
    {
        auto *hashed = reinterpret_cast<const SubpacketSetV6 *>(packet + 1);
        auto hashed_length = hashed->GetLength();

        hashed_block = { hashed->Data, hashed_length };

        auto *unhashed = reinterpret_cast<const SubpacketSetV6 *>(
            reinterpret_cast<const uint8_t *>(hashed) + 4 + hashed_length);
        auto unhashed_length = unhashed->GetLength();

        unhashed_block = { unhashed->Data, unhashed_length };

        auto *end = reinterpret_cast<const SignaturePacketEndV6 *>(
            reinterpret_cast<const uint8_t *>(unhashed) + 4 + unhashed_length);

        hash_left_16_bit = { end->HashLeft16Bit, 2 };

        salt_material = { end->Salt, end->SaltLength };

        auto *signature_block = reinterpret_cast<const SignatureBlock *>(
            reinterpret_cast<const uint8_t *>(end + 1) + end->SaltLength);

        signature_material = { signature_block->Signature, ptr + packet_length };

        break;
    }
    default:
        return toolkit::make_error("unsupported packet version {:02x}", packet->Version);
    }

    Signature signature
    {
        .Version = packet->Version,
        .SignatureType = packet->SignatureType,
        .PublicKeyAlgorithm = packet->PublicKeyAlgorithm,
        .HashAlgorithm = packet->HashAlgorithm,

        .HashedBlock = hashed_block,
        .UnhashedBlock = unhashed_block,

        .HashLeft16Bit = hash_left_16_bit,
        .SaltMaterial = salt_material,
        .SignatureMaterial = signature_material,
    };

    for (auto desc : signature.HashedBlock)
    {
        if (auto res = evaluate_subpacket(desc.Data, desc.Length, signature); !res)
        {
            return res;
        }
    }

    for (auto desc : signature.UnhashedBlock)
    {
        if (auto res = evaluate_subpacket(desc.Data, desc.Length, signature); !res)
        {
            return res;
        }
    }

    return signature;
}

toolkit::result<unvm::pgp::FingerprintReference> unvm::pgp::ParseFingerprint(
    const std::span<const uint8_t> signature_buffer)
{
    auto *pointer = signature_buffer.data();
    auto *header = reinterpret_cast<const PacketHeader *>(pointer);

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

    auto *packet = reinterpret_cast<const SignaturePacket *>(
        reinterpret_cast<const uint8_t *>(header) + header_length);
    
    Signature signature;
    if (auto res = ParseSignature(packet, packet_length) >> signature; !res)
    {
        return res;
    }

    FingerprintReference reference
    {
        .Fingerprint = signature.IssuerFingerprint,
        .KeyID = signature.IssuerKeyID,
        .SignatureType = signature.SignatureType,
        .HashAlgorithm = signature.HashAlgorithm,
    };

    return reference;
}

const unvm::pgp::PublicKey *unvm::pgp::MatchPublicKey(
    const Keyring &keyring,
    const FingerprintReference &fpr,
    const FlagsT flags)
{
    for (auto &certificate : keyring)
    {
        if (std::span(certificate.Key.Fingerprint) == fpr.Fingerprint)
        {
            for (auto &subkey : certificate.Subkeys)
            {
                for (auto &signature : subkey.Signatures)
                {
                    if (std::span(certificate.Key.Fingerprint) != signature.IssuerFingerprint)
                    {
                        continue;
                    }

                    if ((signature.KeyFlags & flags) != flags)
                    {
                        continue;
                    }

                    return &certificate.Key;
                }
            }
        }

        for (auto &subkey : certificate.Subkeys)
        {
            if (std::span(subkey.Key.Fingerprint) == fpr.Fingerprint)
            {
                for (auto &signature : subkey.Signatures)
                {
                    if (std::span(subkey.Key.Fingerprint) != signature.IssuerFingerprint)
                    {
                        continue;
                    }

                    if ((signature.KeyFlags & flags) != flags)
                    {
                        continue;
                    }

                    return &subkey.Key;
                }
            }
        }
    }

    return nullptr;
}

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
    (void) curve;

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

    OSSL_PARAM params[]
    {
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
