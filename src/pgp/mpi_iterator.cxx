#include <unvm/pgp.hxx>

unvm::pgp::MPIIterator::MPIIterator(const std::span<const uint8_t> block)
    : block(block)
{
}

bool unvm::pgp::MPIIterator::operator!=(const MPIIterator &other) const
{
    return block.size() != other.block.size() || block.data() != other.block.data();
}

std::span<const uint8_t> unvm::pgp::MPIIterator::operator*() const
{
    if (block.empty())
    {
        throw std::runtime_error("block is empty.");
    }

    uint16_t bit_count;
    if (auto res = scalar<2>(block) >> bit_count; !res)
    {
        throw std::runtime_error(res.error());
    }

    const auto byte_count = (bit_count + 7u) / 8u;

    if (block.size() < 2 + byte_count)
    {
        throw std::runtime_error("block is too small.");
    }

    return block.subspan(2, byte_count);
}

unvm::pgp::MPIIterator &unvm::pgp::MPIIterator::operator++()
{
    if (block.empty())
    {
        throw std::runtime_error("block is empty.");
    }

    uint16_t bit_count;
    if (auto res = scalar<2>(block) >> bit_count; !res)
    {
        throw std::runtime_error(res.error());
    }

    const auto byte_count = (bit_count + 7u) / 8u;

    if (block.size() < 2 + byte_count)
    {
        throw std::runtime_error("block is too small.");
    }

    block = block.subspan(2 + byte_count);
    return *this;
}

unvm::pgp::MPIIterator unvm::pgp::MPIIterator::operator++(int)
{
    if (block.empty())
    {
        throw std::runtime_error("block is empty.");
    }

    uint16_t bit_count;
    if (auto res = scalar<2>(block) >> bit_count; !res)
    {
        throw std::runtime_error(res.error());
    }

    const auto byte_count = (bit_count + 7u) / 8u;

    if (block.size() < 2 + byte_count)
    {
        throw std::runtime_error("block is too small.");
    }

    const auto pre = block;
    block = block.subspan(2 + byte_count);
    return pre;
}

toolkit::result<std::span<const uint8_t>> unvm::pgp::MPIIterator::bytes(const size_t byte_count)
{
    if (block.size() < byte_count)
    {
        return toolkit::make_error("block is too small.");
    }

    const auto data = block.subspan(0, byte_count);
    block = block.subspan(byte_count);
    return data;
}

toolkit::result<std::span<const uint8_t>> unvm::pgp::MPIIterator::mpi()
{
    if (block.empty())
    {
        return toolkit::make_error("block is empty.");
    }

    uint16_t bit_count;
    if (auto res = scalar<2>(block) >> bit_count; !res)
    {
        return res;
    }

    const auto byte_count = (bit_count + 7u) / 8u;

    if (block.size() < 2 + byte_count)
    {
        return toolkit::make_error("block is too small.");
    }

    const auto data = block.subspan(2, byte_count);
    block = block.subspan(2 + byte_count);
    return data;
}

toolkit::result<unvm::pgp::CurveOID> unvm::pgp::MPIIterator::curve()
{
    if (block.empty())
    {
        return toolkit::make_error("block is empty.");
    }

    const auto byte_count = block[0];

    if (block.size() < 1 + byte_count)
    {
        return toolkit::make_error("block is too small.");
    }

    const auto oid = block.subspan(1, byte_count);

    block = block.subspan(1 + byte_count);

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

    return toolkit::make_error("unsupported curve oid '{}'.", ToHexString(oid));
}

toolkit::result<unvm::pgp::KDF> unvm::pgp::MPIIterator::kdf()
{
    if (block.empty())
    {
        return toolkit::make_error("block is empty.");
    }

    const auto byte_count = block[0];

    if (block.size() < 1 + byte_count)
    {
        return toolkit::make_error("block is too small.");
    }

    const auto ao = block[1];
    const auto ha = block[2];
    const auto sa = block[3];

    block = block.subspan(1 + byte_count);

    return KDF
    {
        .HashAlgorithm = static_cast<HashAlgorithmID>(ha),
        .SymmetricAlgorithm = static_cast<SymmetricAlgorithmID>(sa),
    };
}
