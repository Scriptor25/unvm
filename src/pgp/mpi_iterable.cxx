#include <unvm/pgp.hxx>

unvm::pgp::MPIIterable::MPIIterable(const std::span<const uint8_t> block)
    : block(block)
{
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterable::begin() const
{
    return block;
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterable::end() const
{
    return block.subspan(block.size());
}
