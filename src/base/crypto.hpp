#pragma once

#include "bytes.hpp"

#include "base/hash.hpp"
#include "base/serialization.hpp"

#include <openssl/pem.h>

#include <filesystem>
#include <iosfwd>
#include <memory>

namespace base
{

class AesKey
{
  public:
    //------------------------
    enum class KeyType : std::size_t
    {
        K256BIT = 32, // 32(bytes)
        K128BIT = 16  // 16(bytes)
    };
    //------------------------
    static constexpr std::size_t iv_cbc_size = 16;
    //------------------------
    AesKey();
    explicit AesKey(KeyType type);
    explicit AesKey(const Bytes& bytes_key);
    AesKey(const AesKey&) = default;
    AesKey(AesKey&&) = default;
    AesKey& operator=(const AesKey&) = default;
    AesKey& operator=(AesKey&&) = default;
    ~AesKey() = default;
    //------------------------
    [[nodiscard]] Bytes toBytes() const;
    [[nodiscard]] Bytes encrypt(const Bytes& data) const;
    [[nodiscard]] Bytes decrypt(const Bytes& data) const;
    //------------------------
    std::size_t size() const;
    //------------------------
    void save(const std::filesystem::path& path);
    static AesKey read(const std::filesystem::path& path);
    //------------------------
  private:
    //------------------------
    KeyType _type;
    Bytes _key;
    FixedBytes<16> _iv;
    //------------------------
    static Bytes generateKey(KeyType type);
    static Bytes generateIv();
    //------------------------
    [[nodiscard]] Bytes encrypt256Aes(const Bytes& data) const;
    [[nodiscard]] Bytes decrypt256Aes(const Bytes& data) const;
    [[nodiscard]] Bytes encrypt128Aes(const Bytes& data) const;
    [[nodiscard]] Bytes decrypt128Aes(const Bytes& data) const;
    //------------------------
};


class Secp256PrivateKey
{
  public:
    static constexpr std::size_t SECP256_PRIVATE_KEY_SIZE = 32;
    static constexpr std::size_t SECP256_SIGNATURE_SIZE = 65;
    static constexpr std::size_t SECP256_PUBLIC_KEY_SIZE = 65;
    using Signature = base::FixedBytes<SECP256_SIGNATURE_SIZE>;
    using PrivateKey = base::FixedBytes<SECP256_PRIVATE_KEY_SIZE>;
    using PublicKey = base::FixedBytes<SECP256_PUBLIC_KEY_SIZE>;
    //---------------------------
    Secp256PrivateKey();
    explicit Secp256PrivateKey(const PrivateKey& private_key_bytes);
    Secp256PrivateKey(const Secp256PrivateKey&) = delete;
    Secp256PrivateKey(Secp256PrivateKey&& other) = default;
    Secp256PrivateKey& operator=(const Secp256PrivateKey&) = delete;
    Secp256PrivateKey& operator=(Secp256PrivateKey&& other) = delete;
    //---------------------------
    bool is_valid() const;
    PublicKey toPublicKey() const;
    //---------------------------
    Signature sign(const base::Bytes& bytes_to_sign) const;
    static PublicKey decodeSignatureToPublicKey(const Signature& signature, const base::Bytes& bytes_to_check);
    //---------------------------
    void save(const std::filesystem::path& path) const;
    static Secp256PrivateKey load(const std::filesystem::path& path);
    //---------------------------
    static Secp256PrivateKey deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    const PrivateKey& getBytes() const;

  private:
    const PrivateKey _secp_key;
};


class KeyVault
{
  public:
    KeyVault() = delete;
    explicit KeyVault(const std::string_view& keys_folder);
    KeyVault(const KeyVault&) = delete;
    KeyVault(KeyVault&&) = default;
    KeyVault& operator=(const KeyVault&) = delete;
    KeyVault& operator=(KeyVault&&) = delete;
    //---------------------------
    ~KeyVault() = default;
    //---------------------------
    const base::Secp256PrivateKey& getKey() const noexcept;
    //---------------------------
  private:
    //---------------------------
    base::Secp256PrivateKey _key;
    //---------------------------
};

} // namespace base