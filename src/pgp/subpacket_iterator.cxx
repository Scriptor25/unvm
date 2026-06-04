#include <unvm/pgp.hxx>

bool unvm::pgp::SubpacketIterator::operator!=(const SubpacketIterator &other) const
{
    return ptr != other.ptr;
}

unvm::pgp::SubpacketDescriptor unvm::pgp::SubpacketIterator::operator*() const
{
    return DescribeSubpacket(ptr);
}

unvm::pgp::SubpacketIterator &unvm::pgp::SubpacketIterator::operator++()
{
    const auto descriptor = DescribeSubpacket(ptr);

    ptr = descriptor.Next;

    return *this;
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketIterator::operator++(int)
{
    const auto descriptor = DescribeSubpacket(ptr);

    auto *pre = ptr;

    ptr = descriptor.Next;

    return { pre };
}
