#pragma once

#include "base/property_tree.hpp"
#include "base/crypto.hpp"

#include <memory>

namespace crypto
{

class KeyVault
{
  public:
    KeyVault() = delete;
    KeyVault(const base::PropertyTree& config);
    KeyVault(const KeyVault&) = delete;
    KeyVault(KeyVault&&) = default;
    KeyVault& operator=(const KeyVault&) = delete;
    KeyVault& operator=(KeyVault&&) = default;
    //---------------------------
    ~KeyVault() = default;
    //---------------------------

  private:
    const base::PropertyTree& _config;
    //---------------------------
    std::unique_ptr<base::RsaPublicKey> _public_key;
    std::unique_ptr<base::RsaPrivateKey> _private_key;
    //---------------------------
};

} // namespace crypto