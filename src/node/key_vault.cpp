#include "key_vault.hpp"

#include "base/log.hpp"

namespace crypto
{

KeyVault::KeyVault(const base::PropertyTree& config) : _config{config}
{
    auto public_key_path = config.get<std::string>("key.public_path");
    auto private_key_path = config.get<std::string>("key.private_path");

    if(std::filesystem::exists(public_key_path) && std::filesystem::exists(private_key_path)) {
        _public_key = std::make_unique<base::RsaPublicKey>(base::RsaPublicKey::read(public_key_path));
        _private_key = std::make_unique<base::RsaPrivateKey>(base::RsaPrivateKey::read(public_key_path));
    }
    else {
        LOG_WARNING << "Keys was not found: public[" << public_key_path << "], private[" << private_key_path << "].";
        static constexpr std::size_t rsa_keys_length = 3422;
        auto keys = base::generateKeys(rsa_keys_length);
        _public_key = std::make_unique<base::RsaPublicKey>(std::move(keys.first));
        _private_key = std::make_unique<base::RsaPrivateKey>(std::move(keys.second));
        LOG_WARNING << "Generated new key pair.";
    }
    LOG_INFO << _public_key->toBytes().toHex();
    // TODO: maybe implement unload to disk mechanic for private key.
}

}