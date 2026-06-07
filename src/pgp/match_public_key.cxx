#include <unvm/pgp.hxx>

const unvm::pgp::PublicKey *unvm::pgp::MatchPublicKey(
    const Keyring &keyring,
    const Signature &signature,
    const FlagsT flags)
{
    const PublicKey *best_candidate{};
    uint32_t best_creation_time{};
    bool best_is_primary{};

    auto collect_key_flags = [&](const std::vector<Signature> &signatures) -> FlagsT
    {
        FlagsT result{};

        for (auto &s : signatures)
        {
            result |= s.KeyFlags;
        }

        return result;
    };

    auto consider_candidate = [&](const PublicKey &key, const FlagsT key_flags, const bool is_primary)
    {
        if (!signature.IssuerFingerprint.empty())
        {
            if (key.Fingerprint != signature.IssuerFingerprint)
            {
                return;
            }
        }
        else if (!signature.IssuerKeyID.empty())
        {
            if (key.KeyID != signature.IssuerKeyID)
            {
                return;
            }
        }

        if ((key_flags & flags) != flags)
        {
            return;
        }

        const auto better =
                !best_candidate
                || (is_primary && !best_is_primary)
                || (is_primary == best_is_primary && key.CreationTime > best_creation_time);

        if (better)
        {
            best_candidate = &key;
            best_creation_time = key.CreationTime;
            best_is_primary = is_primary;
        }
    };

    for (auto &certificate : keyring)
    {
        FlagsT primary_flags{};

        for (auto &user : certificate.Users)
        {
            primary_flags |= collect_key_flags(user.Signatures);
        }

        consider_candidate(certificate.Key, primary_flags, true);

        for (auto &subkey : certificate.Subkeys)
        {
            const auto subkey_flags = collect_key_flags(subkey.Signatures);

            consider_candidate(subkey.Key, subkey_flags, false);
        }
    }

    return best_candidate;
}
