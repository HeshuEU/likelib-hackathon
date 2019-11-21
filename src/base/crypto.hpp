#include "base/bytes.hpp"

#include <openssl/rsa.h>
#include <openssl/evp.h>

#include <filesystem>
#include <memory>

namespace base
{

class PublicRsaKey;
class PrivateRsaKey;
std::pair<PublicRsaKey, PrivateRsaKey> generate(const std::size_t keys_size);

class PublicRsaKey
{
  public:
    //----------------------------------

    friend std::pair<PublicRsaKey, PrivateRsaKey> generate(const std::size_t key_size);

    PublicRsaKey() = delete;
    PublicRsaKey(const Bytes& key);
    PublicRsaKey(const std::filesystem::path& path);
    PublicRsaKey(const PublicRsaKey& another) = default;
    PublicRsaKey(PublicRsaKey&& another) = default;
    PublicRsaKey& operator=(const PublicRsaKey& another) = default;
    PublicRsaKey& operator=(PublicRsaKey&& another) = default;

    //----------------------------------

    Bytes encrypt(const Bytes& message) const;

    Bytes encryptWithAes(const Bytes& message) const;

    Bytes decrypt(const Bytes& encrypted_message) const;

    //----------------------------------

    std::size_t size() const noexcept;

    std::size_t maxEncryptSize() const noexcept;

    //----------------------------------

    Bytes toBytes() const noexcept;

    //----------------------------------

    void save(const std::filesystem::path& path) const;

  private:
    Bytes _public_key;

    std::size_t _size;

    //----------------------------------

    PublicRsaKey(RSA* key);
    PublicRsaKey(EVP_PKEY* key);

    //----------------------------------

    RSA* toRsaKey() const;

    EVP_PKEY* toEvpKey() const;
};

class PrivateRsaKey
{
  public:
    //----------------------------------

    friend std::pair<PublicRsaKey, PrivateRsaKey> generate(const std::size_t key_size);

    PrivateRsaKey() = delete;
    PrivateRsaKey(const Bytes& key);
    PrivateRsaKey(const std::filesystem::path& path);
    PrivateRsaKey(const PrivateRsaKey& another) = default;
    PrivateRsaKey(PrivateRsaKey&& another) = default;
    PrivateRsaKey& operator=(const PrivateRsaKey& another) = default;
    PrivateRsaKey& operator=(PrivateRsaKey&& another) = default;

    //----------------------------------

    Bytes encrypt(const Bytes& message) const;

    Bytes decrypt(const Bytes& encrypted_message) const;

    Bytes decryptWithAes(const Bytes& encrypted_message) const;

    //----------------------------------

    std::size_t size() const noexcept;

    std::size_t maxEncryptSize() const noexcept;

    //----------------------------------

    Bytes toBytes() const noexcept;

    //----------------------------------

    void save(const std::filesystem::path& path) const;

  private:
    Bytes _private_key;

    std::size_t _size;

    //----------------------------------

    PrivateRsaKey(RSA* key);
    PrivateRsaKey(EVP_PKEY* key);

    //----------------------------------

    RSA* toRsaKey() const;

    EVP_PKEY* toEvpKey() const;
};

} // namespace base