#include <unvm/data.hxx>

#include <data/cacert.h>
#include <data/keyring.h>

std::span<const uint8_t> unvm::data::cacert
{
    ::cacert,
    ::cacert + cacert_len,
};

std::span<const uint8_t> unvm::data::keyring
{
    ::keyring,
    ::keyring + keyring_len,
};
