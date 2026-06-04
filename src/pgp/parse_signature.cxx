#include <unvm/pgp.hxx>

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
        if (auto res = ParseSubpacket(desc.Data, desc.Length, signature); !res)
        {
            return res;
        }
    }

    for (auto desc : signature.UnhashedBlock)
    {
        if (auto res = ParseSubpacket(desc.Data, desc.Length, signature); !res)
        {
            return res;
        }
    }

    return signature;
}
