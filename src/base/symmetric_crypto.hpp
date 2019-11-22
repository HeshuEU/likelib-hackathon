#pragma once

#include "base/bytes.hpp"

namespace base
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
    KeyType _type;
    Bytes _key;
    Bytes _iv;

    static Bytes generateKey(KeyType type);

    static Bytes generateIv(KeyType type);
};

}; // namespace base