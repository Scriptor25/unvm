#include <unvm/pgp.hxx>

unvm::pgp::SubpacketDescriptor unvm::pgp::DescribeSubpacket(const uint8_t *subpacket, const uint8_t *end)
{
    if (subpacket + 1 > end)
    {
        return {};
    }

    uint32_t subpacket_length;
    uint8_t length_length;

    if (const auto fst = subpacket[0]; fst < 0xC0)
    {
        subpacket_length = fst;
        length_length = 1;
    }
    else if (fst < 0xFF)
    {
        if (subpacket + 2 > end)
        {
            return {};
        }

        const auto snd = subpacket[1];

        subpacket_length = ((fst - 0xC0) << 8) + snd + 0xC0;
        length_length = 2;
    }
    else // if (fst == 0xFF)
    {
        if (subpacket + 5 > end)
        {
            return {};
        }

        subpacket_length = scalar<4>(&subpacket[1]);
        length_length = 5;
    }

    auto *next = subpacket + length_length + subpacket_length;

    if (next > end)
    {
        return {};
    }

    return {
        .Ptr = subpacket,
        .Next = next,
        .Length = subpacket_length,
        .Data = reinterpret_cast<const Subpacket *>(subpacket + length_length),
    };
}
