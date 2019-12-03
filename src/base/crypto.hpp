#include "base/bytes.hpp"

#include <openssl/pem.h>

#include <filesystem>
#include <memory>

namespace base
{

class RsaPublicKey
{
  public:
    RsaPublicKey(const Bytes& key_word);

    RsaPublicKey(const std::filesystem::path path);

    RsaPublicKey(const RsaPublicKey& another) = default;

    RsaPublicKey(RsaPublicKey&& another) = default;

    RsaPublicKey& operator=(const RsaPublicKey& another) = default;

    RsaPublicKey& operator=(RsaPublicKey&& another) = default;

    Bytes encrypt(const Bytes& message) const;

    Bytes encryptWithtAes(const Bytes& message) const;

    Bytes decrypt(const Bytes& encrypted_message) const;

    std::size_t maxEncryptSize() const noexcept;

    void save(const std::filesystem::path& path) const;

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

    RsaPrivateKey(const std::filesystem::path path);

    RsaPrivateKey(const RsaPrivateKey& another) = default;

    RsaPrivateKey(RsaPrivateKey&& another) = default;

    RsaPrivateKey& operator=(const RsaPrivateKey& another) = default;

    RsaPrivateKey& operator=(RsaPrivateKey&& another) = default;

    Bytes encrypt(const Bytes& message) const;

    Bytes decrypt(const Bytes& encrypted_message) const;

    Bytes decryptWithAes(const Bytes& message) const;

    std::size_t maxEncryptSize() const noexcept;

    void save(const std::filesystem::path& path) const;

    Bytes toBytes() const;

  private:
    static constexpr std::size_t ASYMMETRIC_DIFFERENCE = 11;

    std::unique_ptr<RSA, decltype(&::RSA_free)> _rsa_key;
    std::size_t _encrypted_message_size;

    std::size_t encryptedMessageSize() const noexcept;

    static std::unique_ptr<RSA, decltype(&::RSA_free)> loadKey(const Bytes& key_word);
};

std::pair<RsaPublicKey, RsaPrivateKey> generateKeys(std::size_t keys_size);

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

} // namespace base