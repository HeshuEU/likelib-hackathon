#include "base/bytes.hpp"

#include <openssl/rsa.h>

#include <memory>

namespace base
{
class Rsa
{
  public:
    //----------------------------------
    // TODO: construct from any key?
    Rsa();
    Rsa(const Rsa&) = delete;
    Rsa(Rsa&& rsa);
    Rsa& operator=(const Rsa&) = delete;
    Rsa& operator=(Rsa&& obj);
    ~Rsa() = default;

    //----------------------------------

    std::string private_encrypt(const std::string& message) const;

    std::string public_encrypt(const std::string& message) const;

    std::string private_decrypt(const std::string& encrypt_message) const;

    std::string public_decrypt(const std::string& encrypt_message) const;

    void save();


  private:
    std::unique_ptr<RSA, decltype(&::RSA_free)> _private_key;
    std::unique_ptr<RSA, decltype(&::RSA_free)> _public_key;
};
} // namespace base