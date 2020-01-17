#include "base/bytes.hpp"

#include <openssl/pem.h>

#include <filesystem>
#include <memory>

namespace base
{

class RsaPublicKey
{
  public:
    //=================
    RsaPublicKey(const Bytes& key_word);
    RsaPublicKey(const RsaPublicKey& another) = default;
    RsaPublicKey(RsaPublicKey&& another) = default;
    RsaPublicKey& operator=(const RsaPublicKey& another) = default;
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
    static RsaPublicKey read(const std::filesystem::path& path);
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

class RsaPrivateKey
{
  public:
    //=================
    RsaPrivateKey() = default;
    RsaPrivateKey(const Bytes& key_word);
    RsaPrivateKey(const RsaPrivateKey& another) = default;
    RsaPrivateKey(RsaPrivateKey&& another) = default;
    RsaPrivateKey& operator=(const RsaPrivateKey& another) = default;
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
    static RsaPrivateKey read(const std::filesystem::path& path);
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

std::pair<RsaPublicKey, RsaPrivateKey> generateKeys(std::size_t keys_size);

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
    AesKey();
    AesKey(KeyType type);
    AesKey(const Bytes& bytes_key);
    AesKey(const AesKey&) = default;
    AesKey(AesKey&&) = default;
    AesKey& operator=(const AesKey&) = default;
    AesKey& operator=(AesKey&&) = default;
    ~AesKey() = default;
    //=================
    Bytes toBytes() const;
    Bytes encrypt(const Bytes& data) const;
    Bytes decrypt(const Bytes& data) const;
    //=================
    std::size_t size() const;
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
    static Bytes generateIv(KeyType type);
    //=================
    Bytes encrypt256Aes(const Bytes& data) const;
    Bytes decrypt256Aes(const Bytes& data) const;
    Bytes encrypt128Aes(const Bytes& data) const;
    Bytes decrypt128Aes(const Bytes& data) const;
    //=================
};

} // namespace base