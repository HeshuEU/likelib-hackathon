#pragma once

#include "base/bytes.hpp"

#include <openssl/pem.h>

#include <memory>

namespace base
{

namespace rsa
{

    class RsaPublicKey
    {
      public:
        RsaPublicKey(const Bytes& key_word);

        RsaPublicKey(const RsaPublicKey& another) = default;

        RsaPublicKey(RsaPublicKey&& another) = default;

        RsaPublicKey& operator=(const RsaPublicKey& another) = default;

        RsaPublicKey& operator=(RsaPublicKey&& another) = default;

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

    class RsaPrivateKey
    {
      public:
        RsaPrivateKey() = default;

        RsaPrivateKey(const Bytes& key_word);

        RsaPrivateKey(const RsaPrivateKey& another) = default;

        RsaPrivateKey(RsaPrivateKey&& another) = default;

        RsaPrivateKey& operator=(const RsaPrivateKey& another) = default;

        RsaPrivateKey& operator=(RsaPrivateKey&& another) = default;

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

    std::pair<RsaPublicKey, RsaPrivateKey> generateKeys(std::size_t keys_size);

} // namespace rsa

} // namespace base
