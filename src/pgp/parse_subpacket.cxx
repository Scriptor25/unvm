#include <unvm/pgp.hxx>

toolkit::result<> unvm::pgp::ParseSubpacket(
    const Subpacket *packet,
    const uint32_t packet_length,
    Signature &signature)
{
    switch (packet->Type)
    {
    case SubpacketTypeID::KeyFlags:
    {
        auto *subpacket = reinterpret_cast<const KeyFlagsSubpacket *>(packet);

        const std::span flags(subpacket->Flags, packet_length - 1);
        for (size_t i = 0; i < flags.size(); ++i)
        {
            signature.KeyFlags |= flags[i] << (i * 8);
        }

        break;
    }
    case SubpacketTypeID::PrimaryUserID:
    {
        auto *subpacket = reinterpret_cast<const PrimaryUserIDSubpacket *>(packet);

        signature.PrimaryUserID = subpacket->Primary == 0x01;

        break;
    }
    case SubpacketTypeID::PreferredSymmetricAlgorithms:
    {
        auto *subpacket = reinterpret_cast<const PreferredSymmetricAlgorithmsSubpacket *>(packet);

        signature.PreferredSymmetricAlgorithms = { subpacket->Algorithms, packet_length - 1 };

        break;
    }
    case SubpacketTypeID::PreferredHashAlgorithms:
    {
        auto *subpacket = reinterpret_cast<const PreferredHashAlgorithmsSubpacket *>(packet);

        signature.PreferredHashAlgorithms = { subpacket->Algorithms, packet_length - 1 };

        break;
    }
    case SubpacketTypeID::PreferredCompressionAlgorithms:
    {
        auto *subpacket = reinterpret_cast<const PreferredCompressionAlgorithmsSubpacket *>(packet);

        signature.PreferredCompressionAlgorithms = { subpacket->Algorithms, packet_length - 1 };

        break;
    }
    case SubpacketTypeID::IssuerFingerprint:
    {
        auto *subpacket = reinterpret_cast<const IssuerFingerprintSubpacket *>(packet);

        std::span<const uint8_t> fingerprint;
        std::span<const uint8_t> key_id;

        switch (subpacket->KeyVersion)
        {
        case 0x04:
            fingerprint = { subpacket->Fingerprint, subpacket->Fingerprint + 20 };
            key_id = fingerprint.subspan(12, 8);
            break;
        case 0x06:
            fingerprint = { subpacket->Fingerprint, subpacket->Fingerprint + 32 };
            key_id = fingerprint.subspan(0, 8);
            break;
        default:
            return toolkit::make_error("unsupported key version {:02x}", subpacket->KeyVersion);
        }

        if (!signature.IssuerKeyID.empty() && signature.IssuerKeyID != key_id)
        {
            return toolkit::make_error(
                "issuer key id mismatch: expected '{}', got '{}'",
                ToHexString(signature.IssuerKeyID),
                ToHexString(key_id));
        }

        signature.IssuerFingerprintKeyVersion = subpacket->KeyVersion;
        signature.IssuerFingerprint = fingerprint;
        signature.IssuerKeyID = key_id;

        break;
    }
    case SubpacketTypeID::IssuerKeyID:
    {
        auto *subpacket = reinterpret_cast<const IssuerKeyIDSubpacket *>(packet);

        std::span key_id(subpacket->KeyID, 8);

        if (signature.IssuerKeyID.empty())
        {
            signature.IssuerKeyID = subpacket->KeyID;
        }
        else if (signature.IssuerKeyID != key_id)
        {
            return toolkit::make_error(
                "issuer key id mismatch: expected '{}', got '{}'",
                ToHexString(signature.IssuerKeyID),
                ToHexString(key_id));
        }

        break;
    }
    case SubpacketTypeID::SignatureCreationTime:
    {
        auto *subpacket = reinterpret_cast<const SignatureCreationTimeSubpacket *>(packet);

        signature.SignatureCreationTime = scalar(subpacket->Time);

        break;
    }
    case SubpacketTypeID::Features:
    {
        auto *subpacket = reinterpret_cast<const FeaturesSubpacket *>(packet);

        const std::span flags(subpacket->Flags, packet_length - 1);
        for (size_t i = 0; i < flags.size(); ++i)
        {
            signature.Features |= flags[i] << (i * 8);
        }

        break;
    }
    case SubpacketTypeID::KeyServerPreferences:
    {
        auto *subpacket = reinterpret_cast<const KeyServerPreferencesSubpacket *>(packet);

        const std::span flags(subpacket->Flags, packet_length - 1);
        for (size_t i = 0; i < flags.size(); ++i)
        {
            signature.KeyServerPreferences |= flags[i] << (i * 8);
        }

        break;
    }
    case SubpacketTypeID::KeyExpirationTime:
    {
        auto *subpacket = reinterpret_cast<const KeyExpirationTimeSubpacket *>(packet);

        signature.KeyExpirationTime = scalar(subpacket->Time);

        break;
    }
    case SubpacketTypeID::EmbeddedSignature:
    {
        auto *subpacket = reinterpret_cast<const EmbeddedSignatureSubpacket *>(packet);

        Signature embedded;
        if (auto res = ParseSignature(&subpacket->Packet, packet_length - 1) >> embedded; !res)
        {
            return res;
        }

        // TODO: use embedded
        (void) embedded;

        break;
    }
    case SubpacketTypeID::ReasonForRevocation:
    {
        auto *subpacket = reinterpret_cast<const ReasonForRevocationSubpacket *>(packet);

        signature.ReasonForRevocationCode = subpacket->Code;
        signature.ReasonForRevocationMessage = { reinterpret_cast<const char *>(subpacket->Reason), packet_length - 2 };

        break;
    }
    default:
        break;
    }

    return {};
}
