#include <unvm/pgp.hxx>

void unvm::pgp::PGPPacketHeader::Parse(
    PacketTagValue &packet_tag,
    uint32_t &packet_length,
    uint8_t &header_length,
    bool &partial) const
{
    if (Tag.NewFormat)
    {
        auto *tag = reinterpret_cast<const PGPPacketTagNewFormat *>(&Tag);
        packet_tag = tag->PacketTag;

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
        auto *tag = reinterpret_cast<const PGPPacketTagOldFormat *>(&Tag);
        packet_tag = tag->PacketTag;

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

unvm::pgp::PGPSubpacketDescriptor unvm::pgp::DescribeSubpacket(const uint8_t *subpacket)
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
        .Data = reinterpret_cast<const PGPSubpacketData *>(subpacket + length_length),
    };
}

bool unvm::pgp::PGPSubpacketSet::iterator::operator!=(const iterator &it) const
{
    return ptr != it.ptr;
}

unvm::pgp::PGPSubpacketDescriptor unvm::pgp::PGPSubpacketSet::iterator::operator*() const
{
    return DescribeSubpacket(ptr);
}

unvm::pgp::PGPSubpacketSet::iterator &
unvm::pgp::PGPSubpacketSet::iterator::operator++()
{
    const auto descriptor = DescribeSubpacket(ptr);

    ptr = descriptor.Next;

    return *this;
}

uint16_t unvm::pgp::PGPSubpacketSet::GetLength() const
{
    return static_cast<uint16_t>(Length[0]) << 8
           | static_cast<uint16_t>(Length[1]);
}

unvm::pgp::PGPSubpacketSet::iterator
unvm::pgp::PGPSubpacketSet::begin() const
{
    return { Data };
}

unvm::pgp::PGPSubpacketSet::iterator unvm::pgp::PGPSubpacketSet::end() const
{
    return { Data + GetLength() };
}
