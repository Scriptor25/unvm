#include <iostream>
#include <unvm/pgp.hxx>

#include <toolkit/defer.hxx>

#include <unordered_map>

std::string_view unvm::pgp::ToString(const PacketTypeID packet_type)
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

    return "?";
}

std::string_view unvm::pgp::ToString(const SubpacketTypeID subpacket_type)
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

    return "?";
}

std::string_view unvm::pgp::ToString(const SignatureTypeID signature_type)
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

    return "?";
}

std::string_view unvm::pgp::ToString(const PublicKeyAlgorithmID algorithm)
{
    static const std::unordered_map<PublicKeyAlgorithmID, const char *> map
    {
        { PublicKeyAlgorithmID::RSA_ES, "RSA (Encrypt and Signature)" },
        { PublicKeyAlgorithmID::RSA_EO, "RSA (Encrypt Only)" },
        { PublicKeyAlgorithmID::RSA_SO, "RSA (Signature Only)" },
        { PublicKeyAlgorithmID::Elgamal, "Elgamal" },
        { PublicKeyAlgorithmID::DSA, "DSA" },
        { PublicKeyAlgorithmID::ECDH, "ECDH" },
        { PublicKeyAlgorithmID::ECDSA, "ECDSA" },
        { PublicKeyAlgorithmID::X25519, "X25519" },
        { PublicKeyAlgorithmID::X448, "X448" },
        { PublicKeyAlgorithmID::Ed25519, "Ed25519" },
        { PublicKeyAlgorithmID::Ed448, "Ed448" },
    };

    if (const auto it = map.find(algorithm); it != map.end())
    {
        return it->second;
    }

    return "?";
}

std::string_view unvm::pgp::ToString(const HashAlgorithmID algorithm)
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

    return "?";
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
            packet_length = static_cast<uint32_t>(Length[1]) << 24
                            | static_cast<uint32_t>(Length[2]) << 16
                            | static_cast<uint32_t>(Length[3]) << 8
                            | static_cast<uint32_t>(Length[4]);
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
            packet_length = static_cast<uint16_t>(Length[0]) << 8
                            | static_cast<uint16_t>(Length[1]);
            partial = false;
            break;
        case 2:
            packet_length = static_cast<uint32_t>(Length[0]) << 24
                            | static_cast<uint32_t>(Length[1]) << 16
                            | static_cast<uint32_t>(Length[2]) << 8
                            | static_cast<uint32_t>(Length[3]);
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
        subpacket_length = static_cast<uint32_t>(subpacket[1]) << 24
                           | static_cast<uint32_t>(subpacket[2]) << 16
                           | static_cast<uint32_t>(subpacket[3]) << 8
                           | static_cast<uint32_t>(subpacket[4]);
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

uint16_t unvm::pgp::SubpacketSetV4::GetLength() const
{
    return static_cast<uint16_t>(Length[0]) << 8 | static_cast<uint16_t>(Length[1]);
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
    return static_cast<uint32_t>(Length[0]) << 24
           | static_cast<uint32_t>(Length[1]) << 16
           | static_cast<uint32_t>(Length[2]) << 8
           | static_cast<uint32_t>(Length[3]);
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV6::begin() const
{
    return { Data };
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV6::end() const
{
    return { Data + GetLength() };
}

toolkit::result<unvm::pgp::Keyring> unvm::pgp::ParseKeyring(const std::span<const uint8_t> buffer)
{
    Keyring keyring;

    auto *next = buffer.data();
    auto *end = buffer.data() + buffer.size();

    while (next != end)
    {
        auto *header = reinterpret_cast<const PacketHeader *>(next);

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
            uint32_t time;
            PublicKeyAlgorithmID algorithm;
            std::span<const uint8_t> key;

            switch (auto *packet_header = reinterpret_cast<const PublicKeyPacket *>(pointer); packet_header->Version)
            {
            case 0x04:
            {
                auto *packet = reinterpret_cast<const PublicKeyPacketV4 *>(packet_header);

                time = static_cast<uint32_t>(packet->CreationTime[0]) << 24
                       | static_cast<uint32_t>(packet->CreationTime[1]) << 16
                       | static_cast<uint32_t>(packet->CreationTime[2]) << 8
                       | static_cast<uint32_t>(packet->CreationTime[3]);
                algorithm = packet->PublicKeyAlgorithm;
                key = { packet->Key, pointer + packet_length };
                break;
            }
            case 0x06:
            {
                auto *packet = reinterpret_cast<const PublicKeyPacketV6 *>(packet_header);

                time = static_cast<uint32_t>(packet->CreationTime[0]) << 24
                       | static_cast<uint32_t>(packet->CreationTime[1]) << 16
                       | static_cast<uint32_t>(packet->CreationTime[2]) << 8
                       | static_cast<uint32_t>(packet->CreationTime[3]);
                algorithm = packet->PublicKeyAlgorithm;

                const auto key_length = static_cast<uint32_t>(packet->KeyLength[0]) << 24
                                        | static_cast<uint32_t>(packet->KeyLength[1]) << 16
                                        | static_cast<uint32_t>(packet->KeyLength[2]) << 8
                                        | static_cast<uint32_t>(packet->KeyLength[3]);
                key = { packet->Key, packet->Key + key_length };
                break;
            }
            default:
                return toolkit::make_error("unsupported packet version {:02x}", packet_header->Version);
            }

            (void) time;
            (void) algorithm;
            (void) key;

            break;
        }
        // record user into certificate
        case PacketTypeID::UserIDPacket:
        {
            const std::string str(pointer, pointer + packet_length);

            std::cerr << str << std::endl;

            break;
        }
        // record subkey into certificate
        case PacketTypeID::PublicSubkeyPacket:
        {
            auto *packet = reinterpret_cast<const PublicKeyPacket *>(pointer);

            (void) packet;

            break;
        }
        // record signature into user or subkey
        case PacketTypeID::SignaturePacket:
        {
            auto *packet = reinterpret_cast<const SignaturePacket *>(pointer);

            (void) packet;

            break;
        }
        default:
            return toolkit::make_error("unsupported packet type {}", packet_type);
        }

        next += header_length + packet_length;
    }

    return keyring;
}

toolkit::result<> unvm::pgp::VerifySignature(
    const std::span<const uint8_t> buffer,
    const std::span<const uint8_t> signature_buffer,
    EVP_PKEY *public_key)
{
    static const std::unordered_map<HashAlgorithmID, const EVP_MD *> hash_algorithms
    {
        { HashAlgorithmID::MD5, EVP_md5() },
        { HashAlgorithmID::SHA1, EVP_sha1() },
        { HashAlgorithmID::RIPEMD160, EVP_ripemd160() },
        { HashAlgorithmID::SHA256, EVP_sha256() },
        { HashAlgorithmID::SHA384, EVP_sha384() },
        { HashAlgorithmID::SHA512, EVP_sha512() },
        { HashAlgorithmID::SHA224, EVP_sha224() },
        { HashAlgorithmID::SHA3_256, EVP_sha3_256() },
        { HashAlgorithmID::SHA3_512, EVP_sha3_512() },
    };

    auto *header = reinterpret_cast<const PacketHeader *>(signature_buffer.data());

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

    if (packet->Version != 0x04)
    {
        return toolkit::make_error("unsupported packet version {:02x}", packet->Version);
    }

    if (packet->SignatureType != SignatureTypeID::Binary)
    {
        return toolkit::make_error("unsupported signature type {}", packet->SignatureType);
    }

    auto *hashed = reinterpret_cast<const SubpacketSetV4 *>(packet + 1);
    const auto hashed_length = hashed->GetLength();

    auto *unhashed = reinterpret_cast<const SubpacketSetV4 *>(
        reinterpret_cast<const uint8_t *>(hashed) + 2 + hashed_length);
    const auto unhashed_length = unhashed->GetLength();

    auto *signature = reinterpret_cast<const SignatureBlock *>(
        reinterpret_cast<const uint8_t *>(unhashed) + 2 + unhashed_length);

    const EVP_MD *md;
    if (const auto it = hash_algorithms.find(packet->HashAlgorithm); it != hash_algorithms.end())
    {
        md = it->second;
    }
    else
    {
        return toolkit::make_error("unsupported hash algorithm {}", packet->HashAlgorithm);
    }

    auto *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return toolkit::make_error("failed to create evp context.");
    }

    auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, public_key) != 1)
    {
        return toolkit::make_error("failed to initialize digest verify context.");
    }

    if (EVP_DigestVerifyUpdate(ctx, buffer.data(), buffer.size()) != 1)
    {
        return toolkit::make_error("failed to update digest verify context.");
    }

    const uint32_t signature_length = 4 + 2 + hashed_length;

    if (EVP_DigestVerifyUpdate(ctx, packet, signature_length) != 1)
    {
        return toolkit::make_error("failed to update digest verify context.");
    }

    const uint8_t trailer[]
    {
        packet->Version,
        0xFF,
        static_cast<uint8_t>(signature_length >> 24 & 0xFF),
        static_cast<uint8_t>(signature_length >> 16 & 0xFF),
        static_cast<uint8_t>(signature_length >> 8 & 0xFF),
        static_cast<uint8_t>(signature_length & 0xFF),
    };

    if (EVP_DigestVerifyUpdate(ctx, trailer, sizeof(trailer)) != 1)
    {
        return toolkit::make_error("failed to update digest verify context.");
    }

    const uint16_t bit_count = static_cast<uint16_t>(signature->Signature[0]) << 8
                               | static_cast<uint16_t>(signature->Signature[1]);

    if (const uint16_t byte_count = (bit_count + 7) / 8;
        EVP_DigestVerifyFinal(ctx, signature->Signature + 2, byte_count) != 1)
    {
        return toolkit::make_error("failed to update digest verify context.");
    }

    return {};
}
