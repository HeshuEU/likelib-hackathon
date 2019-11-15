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
    Rsa(const std::shared_ptr<RSA>& public_key);
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

    //----------------------------------

    void save(const std::filesystem::path& public_path, const std::filesystem::path& private_path = "-1") const;  //TODO: change it

    //----------------------------------

    size_t size() const;

    size_t maxPrivateEncryptSize() const;

    size_t maxPublicEncryptSize() const;

    //----------------------------------

    std::shared_ptr<RSA> getPublicKey() const;

  private:
    std::shared_ptr<RSA> _private_key;  //TODO: maybe store Bytes?
    std::shared_ptr<RSA> _public_key;
};
} // namespace base