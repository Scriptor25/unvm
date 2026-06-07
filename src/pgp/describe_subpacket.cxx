#include <unvm/pgp.hxx>

toolkit::result<unvm::pgp::SubpacketDescriptor> unvm::pgp::DescribeSubpacket(const std::span<const uint8_t> buffer)
{
    if (buffer.empty())
    {
        return toolkit::make_error("buffer is empty.");
    }

    uint32_t subpacket_length;
    uint8_t length_length;

    if (const auto fst = buffer[0]; fst < 0xC0)
    {
        subpacket_length = fst;
        length_length = 1;
    }
    else if (fst < 0xFF)
    {
        if (buffer.size() < 2)
        {
            return toolkit::make_error("buffer is too small.");
        }

        const auto snd = buffer[1];

        subpacket_length = ((fst - 0xC0) << 8) + snd + 0xC0;
        length_length = 2;
    }
    else // if (fst == 0xFF)
    {
        if (buffer.size() < 5)
        {
            return toolkit::make_error("buffer is too small.");
        }

        subpacket_length = scalar<4>(&buffer[1]);
        length_length = 5;
    }

    if (buffer.size() < length_length + subpacket_length)
    {
        return toolkit::make_error("buffer is too small.");
    }

    return SubpacketDescriptor
    {
        .Next = buffer.subspan(length_length + subpacket_length),
        .Length = subpacket_length,
        .Data = reinterpret_cast<const Subpacket *>(buffer.data() + length_length),
    };
}
