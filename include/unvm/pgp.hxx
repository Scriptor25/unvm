#pragma once

#include <toolkit/result.hxx>

#include <openssl/evp.h>

#include <format>
#include <string_view>

namespace unvm::pgp
{
    enum class PacketTypeID : uint8_t
    {
        PublicKeyEncryptedSessionKeyPacket          = 1,
        SignaturePacket                             = 2,
        SymmetricKeyEncryptedSessionKeyPacket       = 3,
        OnePassSignaturePacket                      = 4,
        SecretKeyPacket                             = 5,
        PublicKeyPacket                             = 6,
        SecretSubkeyPacket                          = 7,
        CompressedDataPacket                        = 8,
        SymmetricallyEncryptedDataPacket            = 9,
        MarkerPacket                                = 10,
        LiteralDataPacket                           = 11,
        TrustPacket                                 = 12,
        UserIDPacket                                = 13,
        PublicSubkeyPacket                          = 14,
        UserAttributePacket                         = 17,
        SymEncryptedAndIntegrityProtectedDataPacket = 18,
        ModificationDetectionCodePacket             = 19,
    };

    enum class SubpacketTypeID : uint8_t
    {
        SignatureCreationTime          = 2,
        SignatureExpirationTime        = 3,
        ExportableCertification        = 4,
        TrustSignature                 = 5,
        RegularExpression              = 6,
        Revocable                      = 7,
        KeyExpirationTime              = 9,
        PreferredSymmetricAlgorithms   = 11,
        RevocationKey                  = 12,
        IssuerKeyID                    = 16,
        NotationData                   = 20,
        PreferredHashAlgorithms        = 21,
        PreferredCompressionAlgorithms = 22,
        KeyServerPreferences           = 23,
        PreferredKeyServer             = 24,
        PrimaryUserID                  = 25,
        PolicyURI                      = 26,
        KeyFlags                       = 27,
        SignersUserID                  = 28,
        ReasonForRevocation            = 29,
        Features                       = 30,
        SignatureTarget                = 31,
        EmbeddedSignature              = 32,
        IssuerFingerprint              = 33,
        IntendedRecipientFingerprint   = 35,
        PreferredAEADCipherSuites      = 39,
    };

    enum class SignatureTypeID : uint8_t
    {
        Binary     = 0x00,
        Text       = 0x01,
        Standalone = 0x02,

        GenericCertification  = 0x10,
        PersonaCertification  = 0x11,
        CasualCertification   = 0x12,
        PositiveCertification = 0x13,

        SubkeyBinding     = 0x18,
        PrimaryKeyBinding = 0x19,

        DirectKey     = 0x1F,
        KeyRevocation = 0x20,

        SubkeyRevocation = 0x28,

        CertificationRevocation = 0x30,

        Timestamp = 0x40,

        ThirdPartyConfirmation = 0x50,
    };

    enum class PublicKeyAlgorithmID : uint8_t
    {
        RSA_ES = 0x01,
        RSA_EO = 0x02,
        RSA_SO = 0x03,

        Elgamal = 0x10,
        DSA     = 0x11,
        ECDH    = 0x12,
        ECDSA   = 0x13,

        X25519  = 0x19,
        X448    = 0x1A,
        Ed25519 = 0x1B,
        Ed448   = 0x1C,
    };

    enum class HashAlgorithmID : uint8_t
    {
        MD5       = 0x01,
        SHA1      = 0x02,
        RIPEMD160 = 0x03,
        SHA256    = 0x08,
        SHA384    = 0x09,
        SHA512    = 0x0A,
        SHA224    = 0x0B,
        SHA3_256  = 0x0C,
        SHA3_512  = 0x0E,
    };

    std::string_view ToString(PacketTypeID packet_type);
    std::string_view ToString(SubpacketTypeID subpacket_type);
    std::string_view ToString(SignatureTypeID signature_type);
    std::string_view ToString(PublicKeyAlgorithmID algorithm);
    std::string_view ToString(HashAlgorithmID algorithm);

    constexpr uint8_t SIG_MD5[]
    {
        0x2A,
        0x86,
        0x48,
        0x86,
        0xF7,
        0x0D,
        0x02,
        0x05,
    };

    constexpr uint8_t SIG_SHA1[]
    {
        0x2B,
        0x0E,
        0x03,
        0x02,
        0x1A,
    };

    constexpr uint8_t SIG_RIPEMD160[]
    {
        0x2B,
        0x24,
        0x03,
        0x02,
        0x01,
    };

    constexpr uint8_t SIG_SHA224[]
    {
        0x60,
        0x86,
        0x48,
        0x01,
        0x65,
        0x03,
        0x04,
        0x02,
        0x04,
    };

    constexpr uint8_t SIG_SHA256[]
    {
        0x60,
        0x86,
        0x48,
        0x01,
        0x65,
        0x03,
        0x04,
        0x02,
        0x01,
    };

    constexpr uint8_t SIG_SHA384[]
    {
        0x60,
        0x86,
        0x48,
        0x01,
        0x65,
        0x03,
        0x04,
        0x02,
        0x02,
    };

    constexpr uint8_t SIG_SHA512[]
    {
        0x60,
        0x86,
        0x48,
        0x01,
        0x65,
        0x03,
        0x04,
        0x02,
        0x03,
    };

    constexpr uint8_t HASH_PREFIX_MD5[]
    {
        0x30,
        0x20,
        0x30,
        0x0C,
        0x06,
        0x08,
        0x2A,
        0x86,
        0x48,
        0x86,
        0xF7,
        0x0D,
        0x02,
        0x05,
        0x05,
        0x00,
        0x04,
        0x10,
    };

    constexpr uint8_t HASH_PREFIX_SHA1[]
    {
        0x30,
        0x21,
        0x30,
        0x09,
        0x06,
        0x05,
        0x2b,
        0x0E,
        0x03,
        0x02,
        0x1A,
        0x05,
        0x00,
        0x04,
        0x14,
    };

    constexpr uint8_t HASH_PREFIX_RIPEMD160[]
    {
        0x30,
        0x21,
        0x30,
        0x09,
        0x06,
        0x05,
        0x2B,
        0x24,
        0x03,
        0x02,
        0x01,
        0x05,
        0x00,
        0x04,
        0x14,
    };

    constexpr uint8_t HASH_PREFIX_SHA224[]
    {
        0x30,
        0x31,
        0x30,
        0x0d,
        0x06,
        0x09,
        0x60,
        0x86,
        0x48,
        0x01,
        0x65,
        0x03,
        0x04,
        0x02,
        0x04,
        0x05,
        0x00,
        0x04,
        0x1C,
    };

    constexpr uint8_t HASH_PREFIX_SHA256[]
    {
        0x30,
        0x31,
        0x30,
        0x0d,
        0x06,
        0x09,
        0x60,
        0x86,
        0x48,
        0x01,
        0x65,
        0x03,
        0x04,
        0x02,
        0x01,
        0x05,
        0x00,
        0x04,
        0x20,
    };

    constexpr uint8_t HASH_PREFIX_SHA384[]
    {
        0x30,
        0x41,
        0x30,
        0x0d,
        0x06,
        0x09,
        0x60,
        0x86,
        0x48,
        0x01,
        0x65,
        0x03,
        0x04,
        0x02,
        0x02,
        0x05,
        0x00,
        0x04,
        0x30,
    };

    constexpr uint8_t HASH_PREFIX_SHA512[]
    {
        0x30,
        0x51,
        0x30,
        0x0d,
        0x06,
        0x09,
        0x60,
        0x86,
        0x48,
        0x01,
        0x65,
        0x03,
        0x04,
        0x02,
        0x03,
        0x05,
        0x00,
        0x04,
        0x40,
    };

    struct SubpacketDescriptor;

#pragma pack(push, 1)
    struct PacketTagLegacy
    {
        uint8_t LengthType : 2;

        PacketTypeID PacketType : 4;

        uint8_t NewFormat : 1;
        uint8_t AlwaysOne : 1;
    };

    struct PacketTag
    {
        PacketTypeID PacketType : 6;
        uint8_t NewFormat       : 1;
        uint8_t AlwaysOne       : 1;
    };

    struct PacketHeader
    {
        void Parse(
            PacketTypeID &packet_type,
            uint32_t &packet_length,
            uint8_t &header_length,
            bool &partial) const;

        PacketTag Tag;
        uint8_t Length[];
    };

    struct PublicKeyPacket
    {
        uint8_t Version;
    };

    struct PublicKeyPacketV4
    {
        uint8_t Version;
        uint8_t CreationTime[4];
        PublicKeyAlgorithmID PublicKeyAlgorithm;

        uint8_t Key[];
    };

    struct PublicKeyPacketV6
    {
        uint8_t Version;
        uint8_t CreationTime[4];
        PublicKeyAlgorithmID PublicKeyAlgorithm;

        uint8_t KeyLength[4];
        uint8_t Key[];
    };

    struct SignaturePacket
    {
        uint8_t Version;
        SignatureTypeID SignatureType;
        PublicKeyAlgorithmID PublicKeyAlgorithm;
        HashAlgorithmID HashAlgorithm;
    };

    struct SubpacketData
    {
        SubpacketTypeID Type;
        uint8_t Data[];
    };

    struct SubpacketIterator
    {
        bool operator!=(const SubpacketIterator &it) const;

        SubpacketDescriptor operator*() const;

        SubpacketIterator &operator++();

        const uint8_t *ptr;
    };

    struct SubpacketSetV4
    {
        [[nodiscard]] uint16_t GetLength() const;

        [[nodiscard]] SubpacketIterator begin() const;
        [[nodiscard]] SubpacketIterator end() const;

        uint8_t Length[2];
        uint8_t Data[];
    };

    struct SubpacketSetV6
    {
        [[nodiscard]] uint32_t GetLength() const;

        [[nodiscard]] SubpacketIterator begin() const;
        [[nodiscard]] SubpacketIterator end() const;

        uint8_t Length[4];
        uint8_t Data[];
    };

    struct SignatureBlock
    {
        uint8_t HashLeft16Bit[2];
        uint8_t Signature[];
    };

    struct IssuerFingerprintSubpacket
    {
        SubpacketTypeID Type;
        uint8_t KeyVersionNumber[1];
        uint8_t Fingerprint[20];
    };

    struct SignatureCreationTimeSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Time[4];
    };

    struct IssuerKeyIDSubpacket
    {
        SubpacketTypeID Type;
        uint8_t KeyID[8];
    };
#pragma pack(pop)

    struct SubpacketDescriptor
    {
        const uint8_t *Subpacket;
        const uint8_t *Next;

        uint32_t Length;
        const SubpacketData *Data;
    };

    SubpacketDescriptor DescribeSubpacket(const uint8_t *subpacket);

    struct Certificate
    {
    };

    using Keyring = std::vector<Certificate>;

    toolkit::result<Keyring> ParseKeyring(std::span<const uint8_t> buffer);

    toolkit::result<> VerifySignature(
        std::span<const uint8_t> buffer,
        std::span<const uint8_t> signature_buffer,
        EVP_PKEY *public_key);
}

template<>
struct std::formatter<unvm::pgp::PacketTypeID>
{
    template<typename C>
    static constexpr auto parse(C &&ctx)
    {
        return ctx.begin();
    }

    template<typename C>
    auto format(const unvm::pgp::PacketTypeID packet_type, C &&ctx) const
    {
        for (auto c : unvm::pgp::ToString(packet_type))
        {
            *ctx.out()++ = c;
        }

        return ctx.out();
    }
};

template<>
struct std::formatter<unvm::pgp::SignatureTypeID>
{
    template<typename C>
    static constexpr auto parse(C &&ctx)
    {
        return ctx.begin();
    }

    template<typename C>
    auto format(const unvm::pgp::SignatureTypeID signature_type, C &&ctx) const
    {
        for (auto c : unvm::pgp::ToString(signature_type))
        {
            *ctx.out()++ = c;
        }

        return ctx.out();
    }
};

template<>
struct std::formatter<unvm::pgp::PublicKeyAlgorithmID>
{
    template<typename C>
    static constexpr auto parse(C &&ctx)
    {
        return ctx.begin();
    }

    template<typename C>
    auto format(const unvm::pgp::PublicKeyAlgorithmID algorithm, C &&ctx) const
    {
        for (auto c : unvm::pgp::ToString(algorithm))
        {
            *ctx.out()++ = c;
        }

        return ctx.out();
    }
};

template<>
struct std::formatter<unvm::pgp::HashAlgorithmID>
{
    template<typename C>
    static constexpr auto parse(C &&ctx)
    {
        return ctx.begin();
    }

    template<typename C>
    auto format(const unvm::pgp::HashAlgorithmID algorithm, C &&ctx) const
    {
        for (auto c : unvm::pgp::ToString(algorithm))
        {
            *ctx.out()++ = c;
        }

        return ctx.out();
    }
};
