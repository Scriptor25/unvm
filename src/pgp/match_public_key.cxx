#include <unvm/pgp.hxx>

const unvm::pgp::PublicKey *unvm::pgp::MatchPublicKey(
    const Keyring &keyring,
    const FingerprintReference &fpr,
    const FlagsT flags)
{
    for (auto &certificate : keyring)
    {
        if (std::span(certificate.Key.Fingerprint) == fpr.Fingerprint)
        {
            for (auto &subkey : certificate.Subkeys)
            {
                for (auto &signature : subkey.Signatures)
                {
                    if (std::span(certificate.Key.Fingerprint) != signature.IssuerFingerprint)
                    {
                        continue;
                    }

                    if ((signature.KeyFlags & flags) != flags)
                    {
                        continue;
                    }

                    return &certificate.Key;
                }
            }
        }

        for (auto &subkey : certificate.Subkeys)
        {
            if (std::span(subkey.Key.Fingerprint) == fpr.Fingerprint)
            {
                for (auto &signature : subkey.Signatures)
                {
                    if (std::span(subkey.Key.Fingerprint) != signature.IssuerFingerprint)
                    {
                        continue;
                    }

                    if ((signature.KeyFlags & flags) != flags)
                    {
                        continue;
                    }

                    return &subkey.Key;
                }
            }
        }
    }

    return nullptr;
}
