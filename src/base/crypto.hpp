#include "base/bytes.hpp"

#include <openssl/rsa.h>

#include <filesystem>
#include <memory>

namespace base
{
class Rsa
{
  public:
    //----------------------------------
    Rsa(const size_t count_bites);
    Rsa(const std::filesystem::path& public_path, const std::filesystem::path& private_path);
    Rsa(const Rsa&) = delete;
    Rsa(Rsa&& rsa);
    Rsa& operator=(const Rsa&) = delete;
    Rsa& operator=(Rsa&& obj);
    ~Rsa() = default;

    //----------------------------------

    Bytes privateEncrypt(const Bytes& message) const;

    Bytes publicEncrypt(const Bytes& message) const;

    Bytes privateDecrypt(const Bytes& encrypt_message) const;

    Bytes publicDecrypt(const Bytes& encrypt_message) const;

    void save(const std::filesystem::path& public_path, const std::filesystem::path& private_path) const;

    size_t size() const;

    size_t maxPrivateEncryptSize() const;

    size_t maxPublicEncryptSize() const;


  private:
    std::unique_ptr<RSA, decltype(&::RSA_free)> _private_key;
    std::unique_ptr<RSA, decltype(&::RSA_free)> _public_key;
};
} // namespace base