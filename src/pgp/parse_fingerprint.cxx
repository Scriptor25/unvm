#include <unvm/pgp.hxx>

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

    auto *packet = reinterpret_cast<const SignaturePacket *>(pointer + header_length);

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
