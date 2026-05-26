#include <unvm/data.hxx>

#include <ssl/cacert.h>
#include <ssl/keyring.h>

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
