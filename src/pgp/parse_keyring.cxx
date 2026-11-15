#include <unvm/pgp.hxx>

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
