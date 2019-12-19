#include "key_vault.hpp"

#include "base/log.hpp"
#include "base/hash.hpp"

namespace crypto
{

KeyVault::KeyVault(const base::PropertyTree& config) : _config{config}
{
    auto public_key_path = config.get<std::string>("keys.public_path");
    auto private_key_path = config.get<std::string>("keys.private_path");

    if(std::filesystem::exists(public_key_path) && std::filesystem::exists(private_key_path)) {
        _public_key = std::make_unique<base::RsaPublicKey>(base::RsaPublicKey::read(public_key_path));
        _private_key = std::make_unique<base::RsaPrivateKey>(base::RsaPrivateKey::read(private_key_path));
    }
    else {
        LOG_WARNING << "Key files was not found: public[" << public_key_path << "], private[" << private_key_path << "].";
        static constexpr std::size_t rsa_keys_length = 1000;
        auto keys = base::generateKeys(rsa_keys_length);
        _public_key = std::make_unique<base::RsaPublicKey>(std::move(keys.first));
        _private_key = std::make_unique<base::RsaPrivateKey>(std::move(keys.second));
        _public_key->save(public_key_path);
        _private_key->save(private_key_path);
        LOG_WARNING << "Generated new key pair and saved by config paths.";
    }
    LOG_INFO << "Public key hash:" << base::Sha256::compute(_public_key->toBytes()).toHex();
    // TODO: maybe implement unload to disk mechanic for private key.
}

}