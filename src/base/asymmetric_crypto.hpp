#pragma once

#include "base/bytes.hpp"

namespace base
{

class PublicKey
{
  public:
    PublicKey() = default;

    PublicKey(const Bytes& key_word);

    PublicKey(const PublicKey& another) = default;

    PublicKey(PublicKey&& another) = default;

    PublicKey& operator=(const PublicKey& another) = default;

    PublicKey& operator=(PublicKey&& another) = default;

    Bytes encrypt(const Bytes& message) const;

    Bytes decrypt(const Bytes& encrypted_message) const;

    std::size_t maxEncryptSize() const noexcept;

    std::size_t encryptedMessageSize() const noexcept;

    Bytes toBytes() const noexcept;

  private:
    static constexpr std::size_t ASYMMETRIC_DIFFERENCE = 42;
    Bytes _public_key;
    std::size_t _encrypted_message_size;
};

class PrivateKey
{
  public:
    PrivateKey() = default;

    PrivateKey(const Bytes& key_word);

    PrivateKey(const PrivateKey& another) = default;

    PrivateKey(PrivateKey&& another) = default;

    PrivateKey& operator=(const PrivateKey& another) = default;

    PrivateKey& operator=(PrivateKey&& another) = default;

    Bytes encrypt(const Bytes& message) const;

    Bytes decrypt(const Bytes& encrypted_message) const;

    std::size_t maxMessageSizeForEncrypt() const noexcept;

    std::size_t encryptedMessageSize() const noexcept;

    Bytes toBytes() const noexcept;

  private:
    static constexpr std::size_t ASYMMETRIC_DIFFERENCE = 11;

    Bytes _private_key;
    std::size_t _encrypted_message_size;
};

std::pair<PublicKey, PrivateKey> generateRsaKeys(std::size_t keys_size);

} // namespace base
