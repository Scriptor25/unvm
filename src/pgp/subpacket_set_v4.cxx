#include <unvm/pgp.hxx>

uint16_t unvm::pgp::SubpacketSetV4::GetLength() const
{
    return scalar(Length);
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV4::begin() const
{
    return { Data };
}

unvm::pgp::SubpacketIterator unvm::pgp::SubpacketSetV4::end() const
{
    return { Data + GetLength() };
}
