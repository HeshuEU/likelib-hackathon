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

    class Key
    {
      public:
        Key();

        Key(KeyType type);

        Key(const Bytes& bytes_key);

        ~Key() = default;

        Bytes toBytes() const;

        Bytes encrypt(const Bytes& data) const;

        Bytes decrypt(const Bytes& data) const;

      private:
        KeyType _type;
        Bytes _key;
        Bytes _iv;

        static Bytes generateKey(KeyType type);

        static Bytes generateIv(KeyType type);
    };

} // namespace aes

}; // namespace base