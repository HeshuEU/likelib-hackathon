#pragma once

#include "base/bytes.hpp"

#include <openssl/pem.h>

#include <memory>

namespace base
{

namespace rsa
{

    class PublicKey
    {
      public:
        PublicKey(const Bytes& key_word);

        PublicKey(const PublicKey& another) = default;

        PublicKey(PublicKey&& another) = default;

        PublicKey& operator=(const PublicKey& another) = default;

        PublicKey& operator=(PublicKey&& another) = default;

        Bytes encrypt(const Bytes& message) const;

        Bytes encryptWithtAes(const Bytes& message) const;

        Bytes decrypt(const Bytes& encrypted_message) const;

        std::size_t maxEncryptSize() const noexcept;

        Bytes toBytes() const;

      private:
        static constexpr std::size_t ASYMMETRIC_DIFFERENCE = 42;

        std::unique_ptr<RSA, decltype(&::RSA_free)> _rsa_key;
        std::size_t _encrypted_message_size;

        std::size_t encryptedMessageSize() const noexcept;

        static std::unique_ptr<RSA, decltype(&::RSA_free)> loadKey(const Bytes& key_word);
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

        Bytes dectyptWithAes(const Bytes& message) const;

        std::size_t maxMessageSizeForEncrypt() const noexcept;

        Bytes toBytes() const;

      private:
        static constexpr std::size_t ASYMMETRIC_DIFFERENCE = 11;

        std::unique_ptr<RSA, decltype(&::RSA_free)> _rsa_key;
        std::size_t _encrypted_message_size;

        std::size_t encryptedMessageSize() const noexcept;

        static std::unique_ptr<RSA, decltype(&::RSA_free)> loadKey(const Bytes& key_word);
    };

    std::pair<PublicKey, PrivateKey> generateKeys(std::size_t keys_size);

} // namespace rsa

} // namespace base
