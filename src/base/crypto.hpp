#pragma once

#include "base/bytes.hpp"
#include "base/property_tree.hpp"
#include "base/serialization.hpp"

#include <openssl/pem.h>

#include <filesystem>
#include <iosfwd>
#include <memory>

namespace base
{

class RsaPublicKey
{
  public:
    //=================
    RsaPublicKey(const Bytes& key_word);
    RsaPublicKey(const RsaPublicKey& another);
    RsaPublicKey(RsaPublicKey&& another) = default;
    RsaPublicKey& operator=(const RsaPublicKey& another);
    RsaPublicKey& operator=(RsaPublicKey&& another) = default;
    //=================
    Bytes encrypt(const Bytes& message) const;
    Bytes encryptWithAes(const Bytes& message) const;
    Bytes decrypt(const Bytes& encrypted_message) const;
    //=================
    std::size_t maxEncryptSize() const noexcept;
    //=================
    Bytes toBytes() const;
    void save(const std::filesystem::path& path) const;
    static RsaPublicKey load(const std::filesystem::path& path);
    //=================
    static RsaPublicKey deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=================
  private:
    //=================
    static constexpr std::size_t ASYMMETRIC_DIFFERENCE = 42;
    //=================
    std::unique_ptr<RSA, decltype(&::RSA_free)> _rsa_key;
    std::size_t _encrypted_message_size;
    //=================
    static std::unique_ptr<RSA, decltype(&::RSA_free)> loadKey(const Bytes& key_word);
    //=================
};

std::ostream& operator<<(std::ostream& os, const RsaPublicKey& public_key);

class RsaPrivateKey
{
  public:
    //=================
    RsaPrivateKey(const Bytes& key_word);
    RsaPrivateKey(const RsaPrivateKey& another) = delete;
    RsaPrivateKey(RsaPrivateKey&& another) = default;
    RsaPrivateKey& operator=(const RsaPrivateKey& another) = delete;
    RsaPrivateKey& operator=(RsaPrivateKey&& another) = default;
    //=================
    Bytes encrypt(const Bytes& message) const;
    Bytes decrypt(const Bytes& encrypted_message) const;
    Bytes decryptWithAes(const Bytes& message) const;
    //=================
    std::size_t maxEncryptSize() const noexcept;
    //=================
    Bytes toBytes() const;
    void save(const std::filesystem::path& path) const;
    static RsaPrivateKey load(const std::filesystem::path& path);
    //=================
  private:
    //=================
    static constexpr std::size_t ASYMMETRIC_DIFFERENCE = 11;
    //=================
    std::unique_ptr<RSA, decltype(&::RSA_free)> _rsa_key;
    std::size_t _encrypted_message_size;
    //=================
    static std::unique_ptr<RSA, decltype(&::RSA_free)> loadKey(const Bytes& key_word);
    //=================
};

std::pair<RsaPublicKey, RsaPrivateKey> generateKeys(std::size_t key_length = 1024);

class AesKey
{
  public:
    //=================
    enum class KeyType : std::size_t
    {
        K256BIT = 32, // 32(bytes)
        K128BIT = 16 // 16(bytes)

    };
    //=================
    static constexpr std::size_t iv_cbc_size = 16;
    //=================
    AesKey();
    explicit AesKey(KeyType type);
    explicit AesKey(const Bytes& bytes_key);
    AesKey(const AesKey&) = default;
    AesKey(AesKey&&) = default;
    AesKey& operator=(const AesKey&) = default;
    AesKey& operator=(AesKey&&) = default;
    ~AesKey() = default;
    //=================
    [[nodiscard]] Bytes toBytes() const;
    [[nodiscard]] Bytes encrypt(const Bytes& data) const;
    [[nodiscard]] Bytes decrypt(const Bytes& data) const;
    //=================
    [[nodiscard]] std::size_t size() const;
    //=================
    void save(const std::filesystem::path& path);
    static AesKey read(const std::filesystem::path& path);
    //=================
  private:
    //=================
    KeyType _type;
    Bytes _key;
    Bytes _iv;
    //=================
    static Bytes generateKey(KeyType type);
    static Bytes generateIv();
    //=================
    [[nodiscard]] Bytes encrypt256Aes(const Bytes& data) const;
    [[nodiscard]] Bytes decrypt256Aes(const Bytes& data) const;
    [[nodiscard]] Bytes encrypt128Aes(const Bytes& data) const;
    [[nodiscard]] Bytes decrypt128Aes(const Bytes& data) const;
    //=================
};


std::string base64Encode(const base::Bytes& bytes);
base::Bytes base64Decode(std::string_view base64);


class KeyVault
{
  public:
    KeyVault() = delete;
    explicit KeyVault(const base::PropertyTree& config);
    KeyVault(const KeyVault&) = delete;
    KeyVault(KeyVault&&) = default;
    KeyVault& operator=(const KeyVault&) = delete;
    KeyVault& operator=(KeyVault&&) = default;
    //---------------------------
    ~KeyVault() = default;
    //---------------------------
    [[nodiscard]] const base::RsaPublicKey& getPublicKey() const noexcept;
    [[nodiscard]] const base::RsaPrivateKey& getPrivateKey() const noexcept;
    //---------------------------
  private:
    //---------------------------
    std::unique_ptr<base::RsaPublicKey> _public_key;
    std::unique_ptr<base::RsaPrivateKey> _private_key;
    //---------------------------
};


} // namespace base