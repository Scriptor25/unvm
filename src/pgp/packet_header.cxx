#include <unvm/pgp.hxx>

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
