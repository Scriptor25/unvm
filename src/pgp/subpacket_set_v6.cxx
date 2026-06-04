#include <unvm/pgp.hxx>

uint32_t unvm::pgp::SubpacketSetV6::GetLength() const
{
    return scalar(Length);
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV6::begin() const
{
    return { Data };
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV6::end() const
{
    return { Data + GetLength() };
}
