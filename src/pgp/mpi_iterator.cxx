#include <unvm/pgp.hxx>

bool unvm::pgp::MPIIterator::operator!=(const MPIIterator &other) const
{
    return ptr != other.ptr;
}

std::span<const uint8_t> unvm::pgp::MPIIterator::operator*() const
{
    if (ptr + 2 > end)
    {
        return {};
    }

    const auto bit_count = scalar<2>(ptr);
    auto byte_count = (bit_count + 7u) / 8u;

    if (ptr + 2 + byte_count > end)
    {
        return {};
    }

    return { ptr + 2, byte_count };
}

unvm::pgp::MPIIterator &unvm::pgp::MPIIterator::operator++()
{
    const auto bit_count = scalar<2>(ptr);
    const auto byte_count = (bit_count + 7u) / 8u;

    ptr += 2 + byte_count;

    return *this;
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterator::operator++(int)
{
    const auto bit_count = scalar<2>(ptr);
    const auto byte_count = (bit_count + 7u) / 8u;

    auto *pre = ptr;

    ptr += 2 + byte_count;

    return { pre };
}

unvm::pgp::CurveOID unvm::pgp::MPIIterator::curve()
{
    if (ptr + 1 > end)
    {
        return CurveOID::Error;
    }

    const auto len = *ptr;

    if (ptr + 1 + len > end)
    {
        return CurveOID::Error;
    }

    const std::span oid(ptr + 1, len);

    ptr += 1 + len;

    if (oid == OID_NIST_P256)
    {
        return CurveOID::NIST_P256;
    }

    if (oid == OID_NIST_P384)
    {
        return CurveOID::NIST_P384;
    }

    if (oid == OID_NIST_P521)
    {
        return CurveOID::NIST_P521;
    }

    if (oid == OID_Brainpool_P256r1)
    {
        return CurveOID::Brainpool_P256r1;
    }

    if (oid == OID_Brainpool_P384r1)
    {
        return CurveOID::Brainpool_P384r1;
    }

    if (oid == OID_Brainpool_P512r1)
    {
        return CurveOID::Brainpool_P512r1;
    }

    if (oid == OID_Ed25519)
    {
        return CurveOID::Ed25519;
    }

    if (oid == OID_Curve25519)
    {
        return CurveOID::Curve25519;
    }

    return CurveOID::Error;
}
