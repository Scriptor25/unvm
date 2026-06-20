#pragma once

#include <unvm/util.hxx>

#include <toolkit/result.hxx>

#include <openssl/evp.h>

#include <format>
#include <span>
#include <string>

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

        Elgamal_EO  = 0x10,
        DSA         = 0x11,
        ECDH        = 0x12,
        ECDSA       = 0x13,
        EdDSALegacy = 0x16,
        X25519      = 0x19,
        X448        = 0x1A,
        Ed25519     = 0x1B,
        Ed448       = 0x1C,
    };

    enum class SymmetricAlgorithmID : uint8_t
    {
        Plain        = 0x00,
        IDEA         = 0x01,
        TripleDES    = 0x02,
        CAST5        = 0x03,
        Blowfish     = 0x04,
        AES_128      = 0x07,
        AES_192      = 0x08,
        AES_256      = 0x09,
        Twofish      = 0x0A,
        Camellia_128 = 0x0B,
        Camellia_192 = 0x0C,
        Camellia_256 = 0x0D,
    };

    enum class CompressionAlgorithmID : uint8_t
    {
        Uncompressed = 0x00,
        ZIP          = 0x01,
        ZLIB         = 0x02,
        BZIP2        = 0x03,
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

    enum class AEADAlgorithmID : uint8_t
    {
        EAX = 0x01,
        OCB = 0x02,
        GCM = 0x03,
    };

    enum class CurveOID : uint8_t
    {
        NIST_P256,
        NIST_P384,
        NIST_P521,
        Brainpool_P256r1,
        Brainpool_P384r1,
        Brainpool_P512r1,
        Ed25519,
        Curve25519,
    };

    constexpr uint8_t OID_NIST_P256[] = { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07 };
    constexpr uint8_t OID_NIST_P384[] = { 0x2B, 0x81, 0x04, 0x00, 0x22 };
    constexpr uint8_t OID_NIST_P521[] = { 0x2B, 0x81, 0x04, 0x00, 0x23 };
    constexpr uint8_t OID_Brainpool_P256r1[] = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 0x07 };
    constexpr uint8_t OID_Brainpool_P384r1[] = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 0x0B };
    constexpr uint8_t OID_Brainpool_P512r1[] = { 0x2B, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 0x0D };
    constexpr uint8_t OID_Ed25519[] = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0xDA, 0x47, 0x0F, 0x01 };
    constexpr uint8_t OID_Curve25519[] = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x97, 0x55, 0x01, 0x05, 0x01 };

    constexpr uint8_t PREFIX_MD5[]
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

    constexpr uint8_t PREFIX_SHA1[]
    {
        0x30,
        0x21,
        0x30,
        0x09,
        0x06,
        0x05,
        0x2B,
        0x0E,
        0x03,
        0x02,
        0x1A,
        0x05,
        0x00,
        0x04,
        0x14,
    };

    constexpr uint8_t PREFIX_RIPEMD160[]
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

    constexpr uint8_t PREFIX_SHA256[]
    {
        0x30,
        0x31,
        0x30,
        0x0D,
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

    constexpr uint8_t PREFIX_SHA384[]
    {
        0x30,
        0x41,
        0x30,
        0x0D,
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

    constexpr uint8_t PREFIX_SHA512[]
    {
        0x30,
        0x51,
        0x30,
        0x0D,
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

    constexpr uint8_t PREFIX_SHA224[]
    {
        0x30,
        0x2D,
        0x30,
        0x0D,
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

    constexpr uint8_t PREFIX_SHA3_256[]
    {
        0x30,
        0x31,
        0x30,
        0x0D,
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
        0x08,
        0x05,
        0x00,
        0x04,
        0x20,
    };

    constexpr uint8_t PREFIX_SHA3_512[]
    {
        0x30,
        0x51,
        0x30,
        0x0D,
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
        0x0a,
        0x05,
        0x00,
        0x04,
        0x40,
    };

    std::string ToString(PacketTypeID packet_type);
    std::string ToString(SubpacketTypeID subpacket_type);
    std::string ToString(SignatureTypeID signature_type);
    std::string ToString(PublicKeyAlgorithmID algorithm);
    std::string ToString(HashAlgorithmID algorithm);
    std::string ToString(CompressionAlgorithmID algorithm);
    std::string ToString(SymmetricAlgorithmID algorithm);

    const char *ToString(CurveOID curve);

    std::string ToHexString(uint8_t value);
    std::string ToHexString(std::span<const uint8_t> value);

    struct Subpacket;

    struct SubpacketDescriptor
    {
        std::span<const uint8_t> Next;

        uint32_t Length;
        const Subpacket *Data;
    };

    struct SubpacketIterator
    {
        SubpacketIterator(std::span<const uint8_t> block);

        bool operator!=(const SubpacketIterator &other) const;

        SubpacketDescriptor operator*() const;

        SubpacketIterator &operator++();
        SubpacketIterator operator++(int);

        std::span<const uint8_t> block;
    };

    struct SubpacketIterable
    {
        SubpacketIterable() = default;
        SubpacketIterable(std::span<const uint8_t> block);

        [[nodiscard]] SubpacketIterator begin() const;
        [[nodiscard]] SubpacketIterator end() const;

        [[nodiscard]] size_t size() const;

        std::span<const uint8_t> block;
    };

    struct KDF
    {
        HashAlgorithmID HashAlgorithm;
        SymmetricAlgorithmID SymmetricAlgorithm;
    };

    struct MPIIterator
    {
        MPIIterator(std::span<const uint8_t> block);

        bool operator!=(const MPIIterator &other) const;

        std::span<const uint8_t> operator*() const;

        MPIIterator &operator++();
        MPIIterator operator++(int);

        [[nodiscard]] toolkit::result<std::span<const uint8_t>> bytes(size_t byte_count);
        [[nodiscard]] toolkit::result<std::span<const uint8_t>> mpi();
        [[nodiscard]] toolkit::result<CurveOID> curve();
        [[nodiscard]] toolkit::result<KDF> kdf();

        std::span<const uint8_t> block;
    };

    struct MPIIterable
    {
        MPIIterable() = default;
        MPIIterable(std::span<const uint8_t> block);

        [[nodiscard]] MPIIterator begin() const;
        [[nodiscard]] MPIIterator end() const;

        std::span<const uint8_t> block;
    };

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

    struct Subpacket
    {
        SubpacketTypeID Type;
        uint8_t Data[];
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

    struct SignaturePacketEndV4
    {
        uint8_t HashLeft16Bit[2];
    };

    struct SignaturePacketEndV6
    {
        uint8_t HashLeft16Bit[2];
        uint8_t SaltLength;
        uint8_t Salt[];
    };

    struct SignatureBlock
    {
        uint8_t Signature[1];
    };

    /**
     * 5.2.3.11. Signature Creation Time
     *
     * (4-octet time field)
     *
     * The time the signature was made.
     *
     * This subpacket MUST be present in the hashed area.
     *
     * When generating this subpacket, it SHOULD be marked as critical.
     */
    struct SignatureCreationTimeSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Time[4];
    };

    /**
     * 5.2.3.18. Signature Expiration Time
     *
     * (4-octet time field)
     *
     * The validity period of the signature. This is the number of seconds after the Signature Creation Time that the signature expires. If this is not present or has a value of zero, it never expires.
     *
     * When an implementation generates this subpacket, it SHOULD be marked as critical.
     */
    struct SignatureExpirationTimeSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Time[4];
    };

    /**
     * 5.2.3.19. Exportable Certification
     *
     * (1 octet of exportability, 0 for not, 1 for exportable)
     *
     * This subpacket denotes whether a certification signature is "exportable"; it is intended for use by users other than the signature's issuer. The packet body contains a Boolean flag indicating whether the signature is exportable. If this packet is not present, the certification is exportable; it is equivalent to a flag containing a 1.
     *
     * Non-exportable, or "local", certifications are signatures made by a user to mark a key as valid within that user's implementation only.
     *
     * Thus, when an implementation prepares a user's copy of a key for transport to another user (this is the process of "exporting" the key), any local certification signatures are deleted from the key.
     *
     * The receiver of a transported key "imports" it and likewise trims any local certifications. In normal operation, there won't be any local certifications, assuming the import is performed on an exported key. However, there are instances where this can reasonably happen. For example, if an implementation allows keys to be imported from a key database in addition to an exported key, then this situation can arise.
     *
     * Some implementations do not represent the interest of a single user (for example, a key server). Such implementations always trim local certifications from any key they handle.
     *
     * When an implementation generates this subpacket and denotes the signature as non-exportable, the subpacket MUST be marked as critical.
     */
    struct ExportableCertificationSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Exportable;
    };

    /**
     *  5.2.3.21. Trust Signature
     *
     * (1 octet "level" (depth), 1 octet of trust amount)
     *
     * The signer asserts that the key is not only valid but also trustworthy at the specified level. Level 0 has the same meaning as an ordinary validity signature. Level 1 means that the signed key is asserted to be a valid trusted introducer, with the 2nd octet of the body specifying the degree of trust. Level 2 means that the signed key is asserted to be trusted to issue level 1 Trust Signatures; that is, the signed key is a "meta introducer". Generally, a level n Trust Signature asserts that a key is trusted to issue level n-1 Trust Signatures. The trust amount is in a range from 0-255, interpreted such that values less than 120 indicate partial trust and values of 120 or greater indicate complete trust. Implementations SHOULD emit values of 60 for partial trust and 120 for complete trust.
     */
    struct TrustSignatureSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Level;
        uint8_t Amount;
    };

    /**
     * 5.2.3.22. Regular Expression
     *
     * (null-terminated UTF-8 encoded Regular Expression)
     *
     * Used in conjunction with Trust Signature packets (of level > 0) to limit the scope of trust that is extended. Only signatures by the target key on User IDs that match the Regular Expression in the body of this packet have trust extended by the Trust Signature subpacket. The Regular Expression uses the same syntax as Henry Spencer's "almost public domain" Regular Expression [REGEX] package. A description of the syntax is found in Section 8. The Regular Expression matches (or does not match) a sequence of UTF-8-encoded Unicode characters from User IDs. The expression itself is also written with UTF-8 characters.
     *
     * For historical reasons, this subpacket includes a null character (an octet with value zero) after the Regular Expression. When an implementation parses a Regular Expression subpacket, it MUST remove this octet; if it is not present, it MUST reject the subpacket (i.e., ignore the subpacket if it's non-critical and reject the signature if it's critical). When an implementation generates a Regular Expression subpacket, it MUST include the null terminator.
     *
     * When generating this subpacket, it SHOULD be marked as critical.
     */
    struct RegularExpressionSubpacket
    {
        SubpacketTypeID Type;
        uint8_t RegularExpression[];
    };

    /**
     * 5.2.3.20. Revocable
     *
     * (1 octet of revocability, 0 for not, 1 for revocable)
     *
     * A Signature's revocability status. The packet body contains a Boolean flag indicating whether the signature is revocable. Signatures that are not revocable ignore any later Revocation Signatures. They represent the signer's commitment that its signature cannot be revoked for the life of its key. If this packet is not present, the signature is revocable.
     */
    struct RevocableSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Revocable;
    };

    /**
     * 5.2.3.13. Key Expiration Time
     *
     * (4-octet time field)
     *
     * The validity period of the key. This is the number of seconds after the key creation time that the key expires. For a direct or certification self-signature, the key creation time is that of the primary key. For a Subkey Binding signature, the key creation time is that of the subkey. If this is not present or has a value of zero, the key never expires. This is found only on a self-signature.
     *
     * When an implementation generates this subpacket, it SHOULD be marked as critical.
     */
    struct KeyExpirationTimeSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Time[4];
    };

    /**
     * 5.2.3.14. Preferred Symmetric Ciphers for v1 SEIPD
     *
     * (array of 1-octet values)
     *
     * A series of Symmetric Cipher Algorithm IDs indicating how the keyholder prefers to receive the version 1 Symmetrically Encrypted and Integrity Protected Data packet (Section 5.13.1). The subpacket body is an ordered list of octets with the most preferred listed first. It is assumed that only the algorithms listed are supported by the recipient's implementation. Algorithm IDs are defined in Section 9.3. This is only found on a self-signature.
     *
     * When generating a v2 SEIPD packet, this preference list is not relevant. See Section 5.2.3.15 instead.
     */
    struct PreferredSymmetricAlgorithmsSubpacket
    {
        SubpacketTypeID Type;
        SymmetricAlgorithmID Algorithms[];
    };

    /**
     * 5.2.3.12. Issuer Key ID
     *
     * (8-octet Key ID)
     *
     * The OpenPGP Key ID of the key issuing the signature. If the version of that key is greater than 4, this subpacket MUST NOT be included in the signature. For these keys, consider the Issuer Fingerprint subpacket (Section 5.2.3.35) instead.
     */
    struct IssuerKeyIDSubpacket
    {
        SubpacketTypeID Type;
        uint8_t KeyID[8];
    };

    /**
     * 5.2.3.24. Notation Data
     *
     * (4 octets of flags, 2 octets of name length (M), 2 octets of value length (N), M octets of name data, N octets of value data)
     *
     * This subpacket describes a "notation" on the signature that the issuer wishes to make. The notation has a name and a value, each of which are strings of octets. There may be more than one notation in a signature. Notations can be used for any extension the issuer of the signature cares to make. The "flags" field holds 4 octets of flags.
     *
     * All undefined flags MUST be zero. Defined flags are as follows:
     *
     * | Flag Position                             | Shorthand      | Description                  |
     * +-------------------------------------------+----------------+------------------------------+
     * | 0x80000000 (first bit of the first octet) | human-readable | Notation value is UTF-8 text |
     *
     * Notation names are arbitrary strings encoded in UTF-8. They reside in two namespaces: the IETF namespace and the user namespace.
     *
     * The IETF namespace is registered with IANA. These names MUST NOT contain the "@" character (0x40). This is a tag for the user namespace.
     *
     * | Notation Name | Data Type | Allowed Values |
     * +---------------+-----------+----------------+
     * | No registrations at this time.             |
     *
     * This registry is initially empty.
     *
     * Names in the user namespace consist of a UTF-8 string tag followed by "@", followed by a DNS domain name. Note that the tag MUST NOT contain an "@" character. For example, the "sample" tag used by Example Corporation could be "sample@example.com".
     *
     * Names in a user space are owned and controlled by the owners of that domain. Obviously, it's bad form to create a new name in a DNS space that you don't own.
     *
     * Since the user namespace is in the form of an email address, implementers MAY wish to arrange for that address to reach a person who can be consulted about the use of the named tag. Note that due to UTF-8 encoding, not all valid user space name tags are valid email addresses.
     *
     * If there is a critical notation, the criticality applies to that specific notation and not to notations in general.
     */
    struct NotationDataSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Flags[4];
        uint8_t NameLength[2];
        uint8_t ValueLength[2];
        uint8_t Data[];
    };

    /**
     * 5.2.3.16. Preferred Hash Algorithms
     *
     * (array of 1-octet values)
     *
     * Message digest algorithm IDs that indicate which algorithms the keyholder prefers to receive. Like the Preferred AEAD Ciphersuites, the list is ordered. Algorithm IDs are defined in Section 9.5. This is only found on a self-signature.
     */
    struct PreferredHashAlgorithmsSubpacket
    {
        SubpacketTypeID Type;
        HashAlgorithmID Algorithms[];
    };

    /**
     * 5.2.3.17. Preferred Compression Algorithms
     *
     * (array of 1-octet values)
     *
     * Compression algorithm IDs that indicate which algorithms the keyholder prefers to use. Like the Preferred AEAD Ciphersuites, the list is ordered. Algorithm IDs are defined in Section 9.4. A zero, or the absence of this subpacket, denotes that uncompressed data is preferred; the keyholder's implementation might have no compression support available. This is only found on a self-signature.
     */
    struct PreferredCompressionAlgorithmsSubpacket
    {
        SubpacketTypeID Type;
        CompressionAlgorithmID Algorithms[];
    };

    /**
     * 5.2.3.25. Key Server Preferences
     *
     * (N octets of flags)
     *
     * This is a list of 1-bit flags that indicates preferences that the keyholder has about how the key is handled on a key server. All undefined flags MUST be zero.
     * 
     * | Flag    | Shorthand | Definition                                                                                                               |
     * +---------+-----------+--------------------------------------------------------------------------------------------------------------------------+
     * | 0x80... | No-modify | The keyholder requests that this key only be modified or updated by the keyholder or an administrator of the key server. |
     *
     * This is found only on a self-signature.
     */
    struct KeyServerPreferencesSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Flags[];
    };

    /**
     * 5.2.3.26. Preferred Key Server
     *
     * (String)
     *
     * This is a URI of a key server that the keyholder prefers be used for updates. Note that keys with multiple User IDs can have a Preferred Key Server for each User ID. Note also that since this is a URI, the key server can actually be a copy of the key retrieved by https, ftp, http, etc.
     */
    struct PreferredKeyServerSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Data[];
    };

    /**
     * 5.2.3.27. Primary User ID
     *
     * (1 octet, Boolean)
     *
     * This is a flag in a User ID's self-signature that states whether this User ID is the main User ID for this key. It is reasonable for an implementation to resolve ambiguities in preferences, for example, by referring to the Primary User ID. If this flag is absent, its value is zero. If more than one User ID in a key is marked as primary, the implementation may resolve the ambiguity in any way it sees fit, but it is RECOMMENDED that priority be given to the User ID with the most recent self-signature.
     *
     * When appearing on a self-signature on a User ID packet, this subpacket applies only to User ID packets. When appearing on a self-signature on a User Attribute packet, this subpacket applies only to User Attribute packets. That is, there are two different and independent "primaries" -- one for User IDs and one for User Attributes.
     */
    struct PrimaryUserIDSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Primary;
    };

    /**
     * 5.2.3.28. Policy URI
     *
     * (String)
     *
     * This subpacket contains a URI of a document that describes the policy under which the signature was issued.
     */
    struct PolicyURISubpacket
    {
        SubpacketTypeID Type;
        uint8_t Data[];
    };

    /**
     * 5.2.3.29. Key Flags
     *
     * (N octets of flags)
     *
     * This subpacket contains a list of binary flags that hold information about a key. It is a string of octets, and an implementation MUST NOT assume a fixed size, so that it can grow over time. If a list is shorter than an implementation expects, the unstated flags are considered to be zero. The defined flags are as follows:
     *
     * | Flag      | Definition                                                                                                                                            |
     * +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------+
     * | 0x01...   | This key may be used to make User ID certifications (Signature Type IDs 0x10-0x13) or Direct Key signatures (Signature Type ID 0x1F) over other keys. |
     * | 0x02...   | This key may be used to sign data.                                                                                                                    |
     * | 0x04...   | This key may be used to encrypt communications.                                                                                                       |
     * | 0x08...   | This key may be used to encrypt storage.                                                                                                              |
     * | 0x10...   | The private component of this key may have been split by a secret-sharing mechanism.                                                                  |
     * | 0x20...   | This key may be used for authentication.                                                                                                              |
     * | 0x80...   | The private component of this key may be in the possession of more than one person.                                                                   |
     * | 0x0004... | Reserved (ADSK)                                                                                                                                       |
     * | 0x0008... | Reserved (timestamping)                                                                                                                               |
     *
     * Usage notes:
     *
     * The flags in this packet may appear in self-signatures or in certification signatures. They mean different things depending on who is making the statement. For example, a certification signature that has the "sign data" flag is stating that the certification is for that use. On the other hand, the "communications encryption" flag in a self-signature is stating a preference that a given key be used for communications. However, note that determining what is "communications" and what is "storage" is a thorny issue. This decision is left wholly up to the implementation; the authors of this document do not claim any special wisdom on the issue and realize that accepted opinion may change.
     *
     * The "split key" (0x10) and "group key" (0x80) flags are placed on a self-signature only; they are meaningless on a certification signature. They SHOULD be placed only on a Direct Key signature (Type ID 0x1F) or a Subkey Binding signature (Type ID 0x18), one that refers to the key the flag applies to.
     *
     * When an implementation generates this subpacket, it SHOULD be marked as critical.
     */
    struct KeyFlagsSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Flags[];
    };

    /**
     * 5.2.3.30. Signer's User ID
     *
     * (String)
     *
     * This subpacket allows a keyholder to state which User ID is responsible for the signing. Many keyholders use a single key for different purposes, such as business communications as well as personal communications. This subpacket allows such a keyholder to state which of their roles is making a signature.
     *
     * This subpacket is not appropriate to use to refer to a User Attribute packet.
     */
    struct SignersUserIDSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Data[];
    };

    /**
     * 5.2.3.31. Reason for Revocation
     *
     * (1 octet of revocation code, N octets of reason string)
     *
     * This subpacket is used only in Key Revocation and Certification Revocation signatures. It describes the reason why the key or certification was revoked.
     *
     * The first octet contains a machine-readable code that denotes the reason for the revocation:
     *
     * | Code    | Reason                                                                       |
     * +---------+------------------------------------------------------------------------------+
     * |       0 | No reason specified (Key Revocation or Certification Revocation signatures)  |
     * |       1 | Key is superseded (Key Revocation signatures)                                |
     * |       2 | Key material has been compromised (Key Revocation signatures)                |
     * |       3 | Key is retired and no longer used (Key Revocation signatures)                |
     * |      32 | User ID information is no longer valid (Certification Revocation signatures) |
     * | 100-110 | Private Use                                                                  |
     *
     * Following the revocation code is a string of octets that gives information about the Reason for Revocation in human-readable form (UTF-8). The string may be null (of zero length). The length of the subpacket is the length of the reason string plus one. An implementation SHOULD implement this subpacket, include it in all Revocation Signatures, and interpret revocations appropriately. There are important semantic differences between the reasons, and there are thus important reasons for revoking signatures.
     *
     * If a key has been revoked because of a compromise, all signatures created by that key are suspect. However, if it was merely superseded or retired, old signatures are still valid. If the revoked signature is the self-signature for certifying a User ID, a revocation denotes that that user name is no longer in use. Such a signature revocation SHOULD include a Reason for Revocation subpacket containing code 32.
     *
     * Note that any certification may be revoked, including a certification on some other person's key. There are many good reasons for revoking a certification signature, such as the case where the keyholder leaves the employ of a business with an email address. A revoked certification is no longer a part of validity calculations.
     */
    struct ReasonForRevocationSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Code;
        uint8_t Reason[];
    };

    /**
     * 5.2.3.32. Features
     *
     * (N octets of flags)
     *
     * The Features subpacket denotes which advanced OpenPGP features a user's implementation supports. This is so that as features are added to OpenPGP that cannot be backward compatible, a user can state that they can use that feature. The flags are single bits that indicate that a given feature is supported.
     *
     * This subpacket is similar to a preferences subpacket and only appears in a self-signature.
     *
     * An implementation SHOULD NOT use a feature listed when sending to a user who does not state that they can use it, unless the implementation can infer support for the feature from another implementation-dependent mechanism.
     *
     * Defined features are as follows:
     *
     * First octet:
     *
     * | Feature | Definition                                                            | Reference      |
     * +---------+-----------------------------------------------------------------------+----------------+
     * | 0x01... | Version 1 Symmetrically Encrypted and Integrity Protected Data packet | Section 5.13.1 |
     * | 0x02... | Reserved                                                              |                |
     * | 0x04... | Reserved                                                              |                |
     * | 0x08... | Version 2 Symmetrically Encrypted and Integrity Protected Data packet | Section 5.13.2 |
     *
     * If an implementation implements any of the defined features, it SHOULD implement the Features subpacket, too.
     *
     * See Section 13.7 for details about how to use the Features subpacket when generating encryption data.
     */
    struct FeaturesSubpacket
    {
        SubpacketTypeID Type;
        uint8_t Flags[];
    };

    /**
     * 5.2.3.33. Signature Target
     *
     * (1 octet public key algorithm, 1 octet hash algorithm, N octets hash)
     *
     * This subpacket identifies a specific target signature to which a signature refers. For Revocation Signatures, this subpacket provides explicit designation of which signature is being revoked. For a Third-Party Confirmation or Timestamp signature, this designates what signature is signed. All arguments are an identifier of that target signature.
     *
     * The N octets of hash data MUST be the size of the signature's hash. For example, a target signature with a SHA-1 hash MUST have 20 octets of hash data.
     */
    struct SignatureTargetSubpacket
    {
        SubpacketTypeID Type;
        PublicKeyAlgorithmID PublicKeyAlgorithm;
        HashAlgorithmID HashAlgorithm;
        uint8_t Hash[];
    };

    /**
     * 5.2.3.34. Embedded Signature
     *
     * (1 Signature packet body)
     *
     * This subpacket contains a complete Signature packet body as specified in Section 5.2. It is useful when one signature needs to refer to, or be incorporated in, another signature.
     */
    struct EmbeddedSignatureSubpacket
    {
        SubpacketTypeID Type;
        SignaturePacket Packet;
    };

    /**
     * 5.2.3.35. Issuer Fingerprint
     *
     * (1 octet key version number, N octets of fingerprint)
     *
     * The OpenPGP Key fingerprint of the key issuing the signature. This subpacket SHOULD be included in all signatures. If the version of the issuing key is 4 and an Issuer Key ID subpacket (Section 5.2.3.12) is also included in the signature, the Key ID of the Issuer Key ID subpacket MUST match the low 64 bits of the fingerprint.
     *
     * Note that the length N of the fingerprint for a version 4 key is 20 octets; for a version 6 key, N is 32. Since the version of the signature is bound to the version of the key, the version octet here MUST match the version of the signature. If the version octet does not match the signature version, the receiving implementation MUST treat it as a malformed signature (see Section 5.2.5).
     */
    struct IssuerFingerprintSubpacket
    {
        SubpacketTypeID Type;
        uint8_t KeyVersion;
        uint8_t Fingerprint[];
    };

    /**
     * 5.2.3.36. Intended Recipient Fingerprint
     *
     * (1 octet key version number, N octets of fingerprint)
     *
     * The OpenPGP Key fingerprint of the intended recipient primary key. If one or more subpackets of this type are included in a signature, it SHOULD be considered valid only in an encrypted context, where the key it was encrypted to is one of the indicated primary keys or one of their subkeys. This can be used to prevent forwarding a signature outside of its intended, encrypted context (see Section 13.12).
     *
     * Note that the length N of the fingerprint for a version 4 key is 20 octets; for a version 6 key, N is 32.
     *
     * An implementation SHOULD generate this subpacket when creating a signed and encrypted message.
     *
     * When generating this subpacket in a version 6 signature, it SHOULD be marked as critical.
     */
    struct IntendedRecipientFingerprintSubpacket
    {
        SubpacketTypeID Type;
        uint8_t KeyVersion;
        uint8_t Fingerprint[];
    };

    /**
     * 5.2.3.15. Preferred AEAD Ciphersuites
     *
     * (array of pairs of octets indicating Symmetric Cipher and AEAD algorithms)
     *
     * A series of paired algorithm IDs indicating how the keyholder prefers to receive the version 2 Symmetrically Encrypted and Integrity Protected Data packet (Section 5.13.2). Each pair of octets indicates a combination of a symmetric cipher and an AEAD mode that the keyholder prefers to use. The Symmetric Cipher Algorithm ID precedes the AEAD algorithm ID in each pair. The subpacket body is an ordered list of pairs of octets with the most preferred algorithm combination listed first.
     *
     * It is assumed that only the combinations of algorithms listed are supported by the recipient's implementation, with the exception of the mandatory-to-implement combination of AES-128 and OCB. If AES-128 and OCB are not found in the subpacket, it is implicitly listed at the end.
     *
     * AEAD algorithm IDs are listed in Section 9.6. Symmetric Cipher Algorithm IDs are listed in Section 9.3.
     *
     * For example, a subpacket containing the six octets
     *
     * `09 02 09 03 13 02`
     *
     * indicates that the keyholder prefers to receive v2 SEIPD using AES-256 with OCB, then AES-256 with GCM, then Camellia-256 with OCB, and finally the implicit AES-128 with OCB.
     *
     * Note that support for the version 2 Symmetrically Encrypted and Integrity Protected Data packet (Section 5.13.2) in general is indicated by a Features Flag (Section 5.2.3.32).
     *
     * This subpacket is only found on a self-signature.
     *
     * When generating a v1 SEIPD packet, this preference list is not relevant. See Section 5.2.3.14 instead.
     */
    struct PreferredAEADCipherSuitesSubpacket
    {
        SubpacketTypeID Type;

        struct
        {
            SymmetricAlgorithmID SymmetricAlgorithm;
            AEADAlgorithmID AEADAlgorithm;
        } Ciphersuites[];
    };
#pragma pack(pop)

    template<size_t N>
    using scalar_t = std::conditional_t<
        N == 1, uint8_t,
        std::conditional_t<
            N == 2, uint16_t,
            std::conditional_t<
                N == 4, uint32_t,
                std::conditional_t<
                    N == 8, uint64_t,
                    void>
            >
        >
    >;

    template<size_t N>
    auto scalar(const uint8_t (&buffer)[N])
    {
        using S = scalar_t<N>;

        S s{};

        for (size_t i = 0; i < N; ++i)
        {
            s |= static_cast<S>(buffer[i]) << ((N - i - 1) * 8);
        }

        return s;
    }

    template<size_t N>
    auto scalar(const uint8_t *buffer)
    {
        using S = scalar_t<N>;

        S s{};

        for (size_t i = 0; i < N; ++i)
        {
            s |= static_cast<S>(buffer[i]) << ((N - i - 1) * 8);
        }

        return s;
    }

    template<size_t N>
    auto scalar(std::span<const uint8_t> buffer)
    {
        using S = scalar_t<N>;

        if (buffer.size() < N)
        {
            return toolkit::result<S>(toolkit::make_error("buffer is too small."));
        }

        S s{};

        for (size_t i = 0; i < N; ++i)
        {
            s |= static_cast<S>(buffer[i]) << ((N - i - 1) * 8);
        }

        return toolkit::result<S>(s);
    }

    template<typename S>
    auto bytes(const S &s)
    {
        constexpr auto N = sizeof(S);

        std::array<uint8_t, N> buffer;

        for (size_t i = 0; i < N; ++i)
        {
            buffer[i] = s >> ((N - i - 1) * 8) & 0xFF;
        }

        return buffer;
    }

    [[nodiscard]] toolkit::result<SubpacketDescriptor> DescribeSubpacket(std::span<const uint8_t> buffer);

    enum class KeyUsageFlag : uint8_t
    {
        Certify        = 1 << 0,
        Sign           = 1 << 1,
        EncryptMessage = 1 << 2,
        EncryptStorage = 1 << 3,
        Split          = 1 << 4,
        Authentication = 1 << 5,
        Shared         = 1 << 7,
    };

    using FlagsT = uint16_t;

    struct PublicKey
    {
        uint32_t CreationTime;
        PublicKeyAlgorithmID Algorithm;
        std::span<const uint8_t> Material;

        std::vector<uint8_t> Fingerprint;
        std::span<const uint8_t> KeyID;
    };

    struct Signature
    {
        uint8_t Version;
        SignatureTypeID SignatureType;
        PublicKeyAlgorithmID PublicKeyAlgorithm;
        HashAlgorithmID HashAlgorithm;

        SubpacketIterable HashedBlock;
        SubpacketIterable UnhashedBlock;

        std::span<const uint8_t> HashLeft16Bit;
        std::span<const uint8_t> SaltMaterial;

        MPIIterable SignatureMaterial;

        FlagsT KeyFlags;
        FlagsT Features;
        FlagsT KeyServerPreferences;

        uint32_t KeyExpirationTime;
        uint32_t SignatureCreationTime;

        bool PrimaryUserID;

        std::span<const SymmetricAlgorithmID> PreferredSymmetricAlgorithms;
        std::span<const HashAlgorithmID> PreferredHashAlgorithms;
        std::span<const CompressionAlgorithmID> PreferredCompressionAlgorithms;

        uint8_t IssuerFingerprintKeyVersion;
        std::span<const uint8_t> IssuerFingerprint;
        std::span<const uint8_t> IssuerKeyID;

        uint8_t ReasonForRevocationCode;
        std::span<const char> ReasonForRevocationMessage;
    };

    struct User
    {
        std::string Id;

        std::vector<Signature> Signatures;
    };

    struct Subkey
    {
        PublicKey Key;

        std::vector<Signature> Signatures;
    };

    struct Certificate
    {
        PublicKey Key;

        std::vector<User> Users;
        std::vector<Subkey> Subkeys;
    };

    using Keyring = std::vector<Certificate>;

    [[nodiscard]] toolkit::result<Keyring> ParseKeyring(std::span<const uint8_t> buffer);
    [[nodiscard]] toolkit::result<PublicKey> ParsePublicKey(const PublicKeyPacket *packet, uint32_t packet_length);
    [[nodiscard]] toolkit::result<Signature> ParseSignature(const SignaturePacket *packet, uint32_t packet_length);
    [[nodiscard]] toolkit::result<> ParseSubpacket(
        const Subpacket *packet,
        uint32_t packet_length,
        Signature &signature);

    [[nodiscard]] toolkit::result<Signature> ParseSignature(std::span<const uint8_t> buffer);

    const PublicKey *MatchPublicKey(const Keyring &keyring, const Signature &signature, FlagsT flags);

    [[nodiscard]] toolkit::result<std::vector<uint8_t>> NormalizeMPI(
        std::span<const uint8_t> mpi,
        size_t max_size);

    [[nodiscard]] toolkit::result<> VerifySignature(
        const Signature &signature,
        std::span<const uint8_t> data,
        EVP_PKEY *public_key,
        size_t public_key_size);

    [[nodiscard]] toolkit::result<std::vector<uint8_t>> BuildSignatureBuffer(
        const Signature &signature,
        std::span<const uint8_t> data);

    [[nodiscard]] toolkit::result<> VerifySignatureBuffer(
        const Signature &signature,
        std::span<const uint8_t> buffer,
        EVP_PKEY *public_key,
        size_t public_key_size);

    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_RSA(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_DSA(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_Elgamal(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_ECDSA(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_EdDSA(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_ECDH(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_X25519(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_X448(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_Ed25519(std::span<const uint8_t> material);
    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey_Ed448(std::span<const uint8_t> material);

    [[nodiscard]] toolkit::result<EVP_PKEY *> CreateOpenSSLPublicKey(const PublicKey &key);
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
        return unvm::PrintString(ctx, unvm::pgp::ToString(packet_type));
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
        return unvm::PrintString(ctx, unvm::pgp::ToString(signature_type));
    }
};

template<>
struct std::formatter<unvm::pgp::SubpacketTypeID>
{
    template<typename C>
    static constexpr auto parse(C &&ctx)
    {
        return ctx.begin();
    }

    template<typename C>
    auto format(const unvm::pgp::SubpacketTypeID packet_type, C &&ctx) const
    {
        return unvm::PrintString(ctx, unvm::pgp::ToString(packet_type));
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
        return unvm::PrintString(ctx, unvm::pgp::ToString(algorithm));
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
        return unvm::PrintString(ctx, unvm::pgp::ToString(algorithm));
    }
};

template<typename T>
concept iterable = requires(T t)
{
    { t.size() };
    { t.begin() };
    { t.end() };
};

template<iterable L, iterable R>
bool operator==(const L &left, const R &right)
{
    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin());
}

template<typename T>
struct array_traits : std::false_type
{
};

template<typename T, size_t N>
struct array_traits<T[N]> : std::true_type
{
    using type = T;

    static constexpr auto count = N;
};

template<typename T>
concept array = array_traits<T>::value;

template<iterable L, array R>
bool operator==(const L &left, const R &right)
{
    using traits = array_traits<R>;

    return left.size() == traits::count && std::equal(left.begin(), left.end(), right);
}

template<array L, iterable R>
bool operator==(const L &left, const R &right)
{
    using traits = array_traits<L>;

    return traits::count == right.size() && std::equal(left, left + traits::count, right.begin());
}
