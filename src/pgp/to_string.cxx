#include <unvm/pgp.hxx>

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
        { PublicKeyAlgorithmID::RSA_ES, "RSA (Encrypt or Sign)" },
        { PublicKeyAlgorithmID::RSA_EO, "RSA (Encrypt Only)" },
        { PublicKeyAlgorithmID::RSA_SO, "RSA (Sign Only)" },
        { PublicKeyAlgorithmID::Elgamal_EO, "Elgamal (Encrypt Only)" },
        { PublicKeyAlgorithmID::DSA, "DSA" },
        { PublicKeyAlgorithmID::ECDH, "ECDH" },
        { PublicKeyAlgorithmID::ECDSA, "ECDSA" },
        { PublicKeyAlgorithmID::EdDSALegacy, "EdDSALegacy" },
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
    return ToHexString({ &value, 1 });
}

std::string unvm::pgp::ToHexString(const std::span<const uint8_t> value)
{
    std::vector<char> buffer(value.size() * 2);

    for (size_t i = 0; i < value.size(); ++i)
    {
        constexpr char lookup[]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

        buffer[i * 2 + 0] = lookup[value[i] >> 4 & 0xF];
        buffer[i * 2 + 1] = lookup[value[i] & 0xF];
    }

    return { buffer.begin(), buffer.end() };
}
