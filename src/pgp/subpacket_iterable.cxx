#include <unvm/pgp.hxx>

unvm::pgp::SubpacketIterable::SubpacketIterable(const std::span<const uint8_t> block)
    : block(block)
{
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterable::begin() const
{
    return block;
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterable::end() const
{
    return block.subspan(block.size());
}

size_t unvm::pgp::SubpacketIterable::size() const
{
    return block.size();
}
