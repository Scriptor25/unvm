#include <unvm/pgp.hxx>

unvm::pgp::SubpacketIterator::SubpacketIterator(std::span<const uint8_t> block)
    : block(block)
{
}

bool unvm::pgp::SubpacketIterator::operator!=(const SubpacketIterator &other) const
{
    return block.size() != other.block.size() || block.data() != other.block.data();
}

unvm::pgp::SubpacketDescriptor unvm::pgp::SubpacketIterator::operator*() const
{
    SubpacketDescriptor descriptor;
    if (auto res = DescribeSubpacket(block) >> descriptor; !res)
    {
        throw std::runtime_error(res.error());
    }

    return descriptor;
}

unvm::pgp::SubpacketIterator &unvm::pgp::SubpacketIterator::operator++()
{
    SubpacketDescriptor descriptor;
    if (auto res = DescribeSubpacket(block) >> descriptor; !res)
    {
        throw std::runtime_error(res.error());
    }

    block = descriptor.Next;
    return *this;
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterator::operator++(int)
{
    SubpacketDescriptor descriptor;
    if (auto res = DescribeSubpacket(block) >> descriptor; !res)
    {
        throw std::runtime_error(res.error());
    }

    const auto pre = block;
    block = descriptor.Next;
    return pre;
}
