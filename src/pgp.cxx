#include <unvm/pgp.hxx>

#include <toolkit/defer.hxx>
#include <toolkit/result.hxx>

#include <openssl/evp.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
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

std::string unvm::pgp::ToHexString(std::span<const uint8_t> buffer)
{
    std::stringstream stream;

    for (auto octet : buffer)
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

unvm::pgp::SubpacketIterable::SubpacketIterable(const uint8_t *first, const uint8_t *last)
    : first(first),
      last(last)
{
}

unvm::pgp::SubpacketIterable::SubpacketIterable(const std::span<const uint8_t> span)
    : first(span.data()),
      last(span.data() + span.size())
{
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterable::begin() const
{
    return { first };
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterable::end() const
{
    return { last };
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
    unvm::pgp::Certificate *certificate,
    unvm::pgp::User *user,
    unvm::pgp::Subkey *subkey,
    const unvm::pgp::SubpacketData *data,
    uint32_t length,
    unvm::pgp::Signature &signature)
{
    switch (data->Type)
    {
    case unvm::pgp::SubpacketTypeID::IssuerFingerprint:
    {
        auto *packet = reinterpret_cast<const unvm::pgp::IssuerFingerprintSubpacket *>(data);

        std::span<const uint8_t> fingerprint;

        switch (packet->KeyVersion)
        {
        case 0x04:
            fingerprint = { packet->Fingerprint, packet->Fingerprint + 20 };
            break;
        case 0x06:
            fingerprint = { packet->Fingerprint, packet->Fingerprint + 32 };
            break;
        default:
            return toolkit::make_error("unsupported key version {:02x}", packet->KeyVersion);
        }

        signature.IssuerFingerprint = fingerprint;

        break;
    }
    default:
        break;
    }

    return {};
}

template<typename ... A>
toolkit::result<std::vector<uint8_t>> digest(const EVP_MD *type, const A &... args)
{
    auto *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return toolkit::make_error("failed to create context.");
    }

    auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestInit(ctx, type) != 1)
    {
        return toolkit::make_error("failed to initialize digest.");
    }
    
    auto res = (toolkit::result() & ... & [&]() -> toolkit::result<>
    {
        if (EVP_DigestUpdate(ctx, &args, sizeof(A)) != 1)
        {
            return toolkit::make_error("failed to update digest.");
        }

        return {};
    });

    if (!res)
    {
        return res;
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned hash_length = 0;

    if (EVP_DigestFinal_ex(ctx, hash, &hash_length) != 1)
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
            uint32_t creation_time;
            PublicKeyAlgorithmID algorithm;
            std::span<const uint8_t> material;

            std::vector<uint8_t> fingerprint;
            std::span<const uint8_t> key_id;

            switch (auto *packet_header = reinterpret_cast<const PublicKeyPacket *>(pointer); packet_header->Version)
            {
            case 0x04:
            {
                auto *packet = reinterpret_cast<const PublicKeyPacketV4 *>(packet_header);

                creation_time = scalar(packet->CreationTime);
                algorithm = packet->PublicKeyAlgorithm;
                material = { packet->Key, pointer + packet_length };

                if (auto res = digest(EVP_sha1(), uint8_t(0x99), uint16_t(packet_length), std::span(pointer, pointer + packet_length)) >> fingerprint; !res)
                {
                    return res;
                }

                key_id = std::span(&fingerprint[12], 8);

                break;
            }
            case 0x06:
            {
                auto *packet = reinterpret_cast<const PublicKeyPacketV6 *>(packet_header);

                creation_time = scalar(packet->CreationTime);
                algorithm = packet->PublicKeyAlgorithm;

                const auto key_length = scalar(packet->KeyLength);
                material = { packet->Key, packet->Key + key_length };

                if (auto res = digest(EVP_sha256(), 0x9B, packet_length, packet) >> fingerprint; !res)
                {
                    return res;
                }

                key_id = std::span(&fingerprint[0], 8);

                break;
            }
            default:
                return toolkit::make_error("unsupported packet version {:02x}", packet_header->Version);
            }

            auto &certificate = keyring.emplace_back();

            current_certificate = &certificate;
            current_user = {};
            current_subkey = {};

            certificate = {
                .Key = {
                    .CreationTime = creation_time,
                    .Algorithm = algorithm,
                    .Material = material,
                },
                .Fingerprint = std::move(fingerprint),
                .KeyID = key_id,
            };

            const time_t time = creation_time;
            const auto *time_point = gmtime(&time);
            const auto *time_string = std::asctime(time_point);

            std::cerr << "[C] " << ToString(algorithm) << " - " << time_string;

            break;
        }
        // record user into certificate
        case PacketTypeID::UserIDPacket:
        {
            auto &user = current_certificate->Users.emplace_back();

            current_user = &user;
            current_subkey = {};

            user.Id = std::string(pointer, pointer + packet_length);

            std::cerr << "    [U] " << user.Id << std::endl;

            break;
        }
        // record subkey into certificate
        case PacketTypeID::PublicSubkeyPacket:
        {
            uint32_t creation_time;
            PublicKeyAlgorithmID algorithm;
            std::span<const uint8_t> material;

            switch (auto *packet_header = reinterpret_cast<const PublicKeyPacket *>(pointer); packet_header->Version)
            {
            case 0x04:
            {
                auto *packet = reinterpret_cast<const PublicKeyPacketV4 *>(packet_header);

                creation_time = scalar(packet->CreationTime);
                algorithm = packet->PublicKeyAlgorithm;
                material = { packet->Key, pointer + packet_length };

                break;
            }
            case 0x06:
            {
                auto *packet = reinterpret_cast<const PublicKeyPacketV6 *>(packet_header);

                creation_time = scalar(packet->CreationTime);
                algorithm = packet->PublicKeyAlgorithm;

                const auto key_length = scalar(packet->KeyLength);
                material = { packet->Key, packet->Key + key_length };

                break;
            }
            default:
                return toolkit::make_error("unsupported packet version {:02x}", packet_header->Version);
            }

            auto &subkey = current_certificate->Subkeys.emplace_back();

            current_user = {};
            current_subkey = &subkey;

            subkey.Key = {
                .CreationTime = creation_time,
                .Algorithm = algorithm,
                .Material = material,
            };

            const time_t time = creation_time;
            const auto *time_point = gmtime(&time);
            const auto *time_string = std::asctime(time_point);

            std::cerr << "    [S] " << ToString(algorithm) << " - " << time_string;

            break;
        }
        // record signature into user or subkey
        case PacketTypeID::SignaturePacket:
        {
            auto *packet = reinterpret_cast<const SignaturePacket *>(pointer);

            std::span<const uint8_t> hashed_block;
            std::span<const uint8_t> unhashed_block;

            std::span<const uint8_t> hash_left_16_bit;
            std::span<const uint8_t> salt;

            std::span<const uint8_t> signature;

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

                signature = { signature_block->Signature, pointer + packet_length };

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

                salt = { end->Salt, end->SaltLength };

                auto *signature_block = reinterpret_cast<const SignatureBlock *>(reinterpret_cast<const uint8_t *>(end + 1) + end->SaltLength);

                signature = { signature_block->Signature, pointer + packet_length };

                break;
            }
            default:
                return toolkit::make_error("unsupported packet version {:02x}", packet->Version);
            }

            if (!current_user && !current_subkey)
            {
                return toolkit::make_error("illegal signature packet without user or subkey.");
            }

            std::cerr << "        [X] " << ToString(packet->SignatureType) << " - " << ToString(
                packet->PublicKeyAlgorithm) << " - " << ToString(packet->HashAlgorithm) << std::endl;

            std::vector<Signature> *target;
            if (current_user)
            {
                target = &current_user->Signatures;
            }
            else if (current_subkey)
            {
                target = &current_subkey->Signatures;
            }

            auto &entry = target->emplace_back();

            for (auto desc : SubpacketIterable(hashed_block))
            {
                std::cerr << "            [*] " << ToString(desc.Data->Type) << std::endl;

                auto res = evaluate_subpacket(
                    current_certificate,
                    current_user,
                    current_subkey,
                    desc.Data,
                    desc.Length,
                    entry);
                if (!res)
                {
                    return res;
                }
            }

            for (auto desc : SubpacketIterable(unhashed_block))
            {
                std::cerr << "            [~] " << ToString(desc.Data->Type) << std::endl;

                auto res = evaluate_subpacket(
                    current_certificate,
                    current_user,
                    current_subkey,
                    desc.Data,
                    desc.Length,
                    entry);
                if (!res)
                {
                    return res;
                }
            }

            break;
        }
        default:
            return toolkit::make_error("unsupported packet type {}", packet_type);
        }

        next += header_length + packet_length;
    }

    return keyring;
}

toolkit::result<unvm::pgp::FingerprintReference> unvm::pgp::ParseFingerprint(std::span<const uint8_t> signature_buffer)
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

    std::span<const uint8_t> hashed_block;
    std::span<const uint8_t> unhashed_block;

    std::span<const uint8_t> hash_left_16_bit;
    std::span<const uint8_t> salt;

    std::span<const uint8_t> signature;

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

        signature = { signature_block->Signature, pointer + packet_length };

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

        salt = { end->Salt, end->SaltLength };
        
        auto *signature_block = reinterpret_cast<const SignatureBlock *>(reinterpret_cast<const uint8_t *>(end + 1) + end->SaltLength);

        signature = { signature_block->Signature, pointer + packet_length };

        break;
    }
    default:
        return toolkit::make_error("unsupported packet version {:02x}", packet->Version);
    }

    FingerprintReference reference
    {
        .SignatureType = packet->SignatureType,
        .HashAlgorithm = packet->HashAlgorithm,
    };

    for (auto desc : SubpacketIterable(hashed_block))
    {
        switch (desc.Data->Type)
        {
        case SubpacketTypeID::IssuerFingerprint:
        {
            auto *subpacket = reinterpret_cast<const IssuerFingerprintSubpacket *>(desc.Data);

            switch (subpacket->KeyVersion)
            {
            case 0x04:
                reference.Fingerprint = { subpacket->Fingerprint, 20 };
                break;
            case 0x06:
                reference.Fingerprint = { subpacket->Fingerprint, 32 };
                break;
            default:
                return toolkit::make_error("unsupported packet version {:02x}", packet->Version);
            }

            break;
        }
        case SubpacketTypeID::IssuerKeyID:
        {
            auto *subpacket = reinterpret_cast<const IssuerKeyIDSubpacket *>(desc.Data);

            reference.KeyID = subpacket->KeyID;

            break;
        }
        default:
            break;
        }
    }

    return reference;
}

const unvm::pgp::PublicKey *unvm::pgp::MatchPublicKey(const Keyring &keyring, const FingerprintReference &fpr)
{
    for (auto &certificate : keyring)
    {
        if (std::span(certificate.Fingerprint) == fpr.Fingerprint)
        {
            return &certificate.Key;
        }

        for (auto &subkey : certificate.Subkeys)
        {
            for (auto &signature : subkey.Signatures)
            {
                if (signature.IssuerFingerprint == fpr.Fingerprint)
                {
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

    // TODO: add support for v6
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

    auto *end = reinterpret_cast<const SignaturePacketEndV4 *>(reinterpret_cast<const uint8_t *>(unhashed) + 2 + unhashed_length);
    auto *signature = reinterpret_cast<const SignatureBlock *>(end + 1);

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
        return toolkit::make_error("failed to create context.");
    }

    auto guard_ctx = toolkit::defer(EVP_MD_CTX_free, ctx);

    if (EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, public_key) != 1)
    {
        return toolkit::make_error("failed to initialize digest verify.");
    }

    if (EVP_DigestVerifyUpdate(ctx, buffer.data(), buffer.size()) != 1)
    {
        return toolkit::make_error("failed to update digest verify.");
    }

    const uint32_t signature_length = 4 + 2 + hashed_length;

    if (EVP_DigestVerifyUpdate(ctx, packet, signature_length) != 1)
    {
        return toolkit::make_error("failed to update digest verify.");
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
        return toolkit::make_error("failed to update digest verify.");
    }

    const auto bit_count = scalar<2>(signature->Signature);

    if (const auto byte_count = (bit_count + 7u) / 8u;
        EVP_DigestVerifyFinal(ctx, signature->Signature + 2, byte_count) != 1)
    {
        return toolkit::make_error("failed to finalize digest verify.");
    }

    return {};
}
