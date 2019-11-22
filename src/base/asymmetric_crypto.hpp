#pragma once

#include "base/bytes.hpp"

namespace base
{

class PublicKey
{
  public:
    PublicKey(const Bytes& key);

    PublicKey(const PublicKey& another) = default;

    PublicKey(PublicKey&& another) = default;

    PublicKey& operator=(const PublicKey& another) = default;

    PublicKey& operator=(PublicKey&& another) = default;

    Bytes encrypt(const Bytes& message) const;

    Bytes decrypt(const Bytes& encrypted_message) const;

    std::size_t maxEncryptSize() const noexcept;

    std::size_t size() const;

    Bytes toBytes() const noexcept;

  private:
    Bytes _public_key;

};

class PrivateKey
{
  public:
    PrivateKey(const Bytes& key);

    PrivateKey(const PrivateKey& another) = default;

    PrivateKey(PrivateKey&& another) = default;

    PrivateKey& operator=(const PrivateKey& another) = default;

    PrivateKey& operator=(PrivateKey&& another) = default;

    Bytes encrypt(const Bytes& message) const;

    Bytes decrypt(const Bytes& encrypted_message) const;

    std::size_t maxEncryptSize() const noexcept;

    std::size_t size() const;

    Bytes toBytes() const noexcept;

  private:
    Bytes _private_key;
};

std::pair<PublicKey, PrivateKey> generate(std::size_t keys_size);

}

