#pragma once

#include "base/bytes.hpp"

namespace base
{

namespace aes
{

    enum class KeyType
    {
        Aes256BitKey,
        Aes128BitKey
    };

    class AesKey
    {
      public:
        AesKey();

        AesKey(KeyType type);

        AesKey(const Bytes& bytes_key);

        ~AesKey() = default;

        Bytes toBytes() const;

        Bytes encrypt(const Bytes& data) const;

        Bytes decrypt(const Bytes& data) const;

      private:
        static constexpr std::size_t _aes_256_size = 48;
        static constexpr std::size_t _aes_128_size = 24;

        KeyType _type;
        Bytes _key;
        Bytes _iv;

        static Bytes generateKey(KeyType type);

        static Bytes generateIv(KeyType type);

        Bytes encrypt256Aes(const Bytes& data) const;

        Bytes decrypt256Aes(const Bytes& data) const;

        Bytes encrypt128Aes(const Bytes& data) const;

        Bytes decrypt128Aes(const Bytes& data) const;
    };

} // namespace aes

}; // namespace base