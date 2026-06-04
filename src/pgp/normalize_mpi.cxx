#include <unvm/pgp.hxx>

toolkit::result<std::vector<uint8_t>> unvm::pgp::NormalizeMPI(const std::span<const uint8_t> mpi, size_t max_size)
{
    auto *data = mpi.data();
    auto size = mpi.size();

    while (size && !data[0])
    {
        ++data;
        --size;
    }

    if (size > max_size)
    {
        return toolkit::make_error("mpi size ({}) > max size ({})", size, max_size);
    }

    std::vector<uint8_t> buffer(max_size);

    std::copy_n(data, size, buffer.data() + max_size - size);

    return buffer;
}
