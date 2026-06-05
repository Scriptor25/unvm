#include <unvm/pgp.hxx>

bool unvm::pgp::MPIIterator::operator!=(const MPIIterator &other) const
{
    return ptr != other.ptr;
}

std::span<const uint8_t> unvm::pgp::MPIIterator::operator*() const
{
    const auto bit_count = scalar<2>(ptr);
    auto byte_count = (bit_count + 7u) / 8u;

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

std::span<const uint8_t> unvm::pgp::MPIIterator::bytes(size_t n)
{
    auto *p = ptr;

    ptr += n;

    return { p, n };
}

std::span<const uint8_t> unvm::pgp::MPIIterator::mpi()
{
    const auto count = scalar<2>(ptr);
    auto n = (count + 7u) / 8u;

    auto *p = ptr + 2;

    ptr += 2 + n;

    return { p, n };
}

unvm::pgp::CurveOID unvm::pgp::MPIIterator::curve()
{
    const auto len = *ptr;
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

unvm::pgp::KDF unvm::pgp::MPIIterator::kdf()
{
    const auto len = ptr[0];
    const auto ao = ptr[1];
    const auto ha = ptr[2];
    const auto sa = ptr[3];

    ptr += 1 + len;

    return {
        .HashAlgorithm = static_cast<HashAlgorithmID>(ha),
        .SymmetricAlgorithm = static_cast<SymmetricAlgorithmID>(sa),
    };
}
