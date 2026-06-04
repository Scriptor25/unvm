#include <unvm/pgp.hxx>

unvm::pgp::SubpacketIterable::SubpacketIterable(const uint8_t *first, const uint8_t *last)
    : block(first, last)
{
}

unvm::pgp::SubpacketIterable::SubpacketIterable(const std::span<const uint8_t> block)
    : block(block)
{
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterable::begin() const
{
    return { block.data(), block.data() + block.size() };
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterable::end() const
{
    return { block.data() + block.size(), block.data() + block.size() };
}

size_t unvm::pgp::SubpacketIterable::size() const
{
    return block.size();
}
