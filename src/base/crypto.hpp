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

    Bytes private_encrypt(const Bytes& message) const;

    Bytes public_encrypt(const Bytes& message) const;

    Bytes private_decrypt(const Bytes& encrypt_message) const;

    Bytes public_decrypt(const Bytes& encrypt_message) const;

    void save(const std::filesystem::path& public_path, const std::filesystem::path& private_path) const;

    size_t size() const;

    size_t max_private_encrypt_size() const;
    
    size_t max_public_encrypt_size() const;


  private:
    std::unique_ptr<RSA, decltype(&::RSA_free)> _private_key;
    std::unique_ptr<RSA, decltype(&::RSA_free)> _public_key;
};
} // namespace base