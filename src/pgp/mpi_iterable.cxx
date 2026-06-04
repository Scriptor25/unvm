#include <unvm/pgp.hxx>

unvm::pgp::MPIIterable::MPIIterable(const uint8_t *first, const uint8_t *last)
    : block(first, last)
{
}

unvm::pgp::MPIIterable::MPIIterable(const std::span<const uint8_t> block)
    : block(block)
{
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterable::begin() const
{
    return { block.data(), block.data() + block.size() };
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterable::end() const
{
    return { block.data() + block.size(), block.data() + block.size() };
}
