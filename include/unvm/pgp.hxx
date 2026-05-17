#pragma once

#include <cstdint>

namespace unvm::pgp
{
    enum class PacketTagValue : uint8_t
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

    enum class SubpacketTypeValue : uint8_t
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

    enum class SignatureTypeValue : uint8_t
    {
        BinaryDocument        = 0x00,
        CanonicalTextDocument = 0x01,
        Standalone            = 0x02,

        GenericCertificationOfUserIDAndPublicKey  = 0x10,
        PersonaCertificationOfUserIDAndPublicKey  = 0x11,
        CasualCertificationOfUserIDAndPublicKey   = 0x12,
        PositiveCertificationOfUserIDAndPublicKey = 0x13,

        SubkeyBinding     = 0x18,
        PrimaryKeyBinding = 0x19,

        DirectlyOnKey = 0x1F,
        KeyRevocation = 0x20,

        SubkeyRevocation = 0x28,

        CertificationRevocation = 0x30,

        Timestamp = 0x40,

        ThirdPartyConfirmation = 0x50,
    };

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

    constexpr uint8_t SIG_RIPEMD_160[]
    {
        0x2B,
        0x24,
        0x03,
        0x02,
        0x01,
    };

    constexpr uint8_t SIG_SHA_1[]
    {
        0x2B,
        0x0E,
        0x03,
        0x02,
        0x1A,
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

    constexpr uint8_t HASH_PREFIX_RIPEMD_160[]
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

    constexpr uint8_t HASH_PREFIX_SHA_1[]
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

#pragma pack(push, 1)
    /**
     * - [5:2] - packet tag
     * - [1:0] - length type
     */
    struct PGPPacketTagOldFormat
    {
        /**
         * 0 - one-octet length
         * 1 - two-octet length
         * 2 - four-octet length
         * 3 - indeterminate length
         */
        uint8_t LengthType : 2;

        PacketTagValue PacketTag : 4;

        uint8_t NewFormat : 1;
        uint8_t AlwaysOne : 1;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    /**
     * - [5:0] - packet tag
     *
     * length type:
     *  - up to 0xBF: one-octet length
     *  - up to 0x20BF: two-octet length
     *  - up to 0xFFFFFFFF: four-octet-length
     *  - otherwise indeterminate length
     */
    struct PGPPacketTagNewFormat
    {
        PacketTagValue PacketTag : 6;
        uint8_t NewFormat        : 1;
        uint8_t AlwaysOne        : 1;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    /**
     * - [7] - always one
     * - [6] - new format
     * - [5:0] - packet tag
     */
    struct PGPPacketTag
    {
        uint8_t           : 6;
        uint8_t NewFormat : 1;
        uint8_t AlwaysOne : 1;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct PGPPacketHeader
    {
        void Parse(
            PacketTagValue &packet_tag,
            uint32_t &packet_length,
            uint8_t &header_length,
            bool &partial) const;

        PGPPacketTag Tag;
        uint8_t Length[];
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct PGPVersion4SignaturePacket
    {
        /**
         * One-octet version number (4).
         */
        uint8_t Version;

        /**
         * One-octet signature type.
         */
        SignatureTypeValue SignatureType;

        /**
         * One-octet public-key algorithm.
         */
        uint8_t PublicKeyAlgorithm;

        /**
         * One-octet hash algorithm.
         */
        uint8_t HashAlgorithm;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct PGPSubpacketData
    {
        SubpacketTypeValue Type;
        uint8_t Data[];
    };
#pragma pack(pop)

    struct PGPSubpacketDescriptor
    {
        const uint8_t *Subpacket;
        const uint8_t *Next;

        uint32_t Length;
        const PGPSubpacketData *Data;
    };

    PGPSubpacketDescriptor DescribeSubpacket(const uint8_t *subpacket);

#pragma pack(push, 1)
    struct PGPSubpacketSet
    {
        struct iterator
        {
            bool operator!=(const iterator &it) const;

            PGPSubpacketDescriptor operator*() const;

            iterator &operator++();

            const uint8_t *ptr;
        };

        [[nodiscard]] uint16_t GetLength() const;

        [[nodiscard]] iterator begin() const;
        [[nodiscard]] iterator end() const;

        /**
         * Two-octet scalar octet count for following hashed subpacket data. Note that this is the length in octets of
         * all the hashed subpackets; a pointer incremented by this number will skip over the hashed subpackets.
         */
        uint8_t Length[2];

        /**
         * Hashed subpacket data set (zero or more subpackets).
         */
        uint8_t Data[];
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct PGPSignatureBlock
    {
        /**
         * Two-octet field holding the left 16 bits of the signed hash value.
         */
        uint8_t HashLeft16Bit[2];

        /**
         * One or more multiprecision integers comprising the signature. This portion is algorithm specific, as
         * described above.
         */
        uint8_t Signature[];
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct PGPSignatureCreationTimeSubpacket
    {
        SubpacketTypeValue Type;
        uint8_t Data[4];
    };
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)
}
