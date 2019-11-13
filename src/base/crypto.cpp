#include "crypto.hpp"

#include "base/assert.hpp"

#include <openssl/bn.h>
#include <openssl/pem.h>

namespace base
{

Rsa::Rsa() : _private_key(RSA_new(), ::RSA_free), _public_key(RSA_new(), ::RSA_free)
{
    std::unique_ptr<BIGNUM, decltype(&::BN_free)> bn(BN_new(), ::BN_free);
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa(RSA_new(), ::RSA_free);
    BN_set_word(bn.get(), RSA_F4);
    RSA_generate_key_ex(rsa.get(), 4096, bn.get(), NULL);

    _private_key = std::unique_ptr<RSA, decltype(&::RSA_free)>(RSAPrivateKey_dup(rsa.get()), ::RSA_free);
    _public_key = std::unique_ptr<RSA, decltype(&::RSA_free)>(RSAPublicKey_dup(rsa.get()), ::RSA_free);
}


Rsa::Rsa(const std::filesystem::path& public_path, const std::filesystem::path& private_path)
    : _private_key(RSA_new(), ::RSA_free), _public_key(RSA_new(), ::RSA_free)
{
    std::unique_ptr<FILE> public_file(fopen(public_path.c_str(), "rb"));
    std::unique_ptr<FILE> private_file(fopen(private_path.c_str(), "rb"));
    RSA* private_temp = _private_key.get();     //TODO: Fix it
    RSA* public_temp = _public_key.get();
    PEM_read_RSAPrivateKey(private_file.get(), &private_temp, NULL, NULL);
    PEM_read_RSAPublicKey(public_file.get(), &public_temp, NULL, NULL);
}


Rsa::Rsa(Rsa&& object) : _private_key(std::move(object._private_key)), _public_key(std::move(object._public_key))
{}


Rsa& Rsa::operator=(Rsa&& object)
{
    if(this == &object) {
        return *this;
    }
    _private_key = std::move(object._private_key);
    _public_key = std::move(object._public_key);

    return *this;
}


Bytes Rsa::private_encrypt(const Bytes& message) const
{
    ASSERT(message.size() <= RSA_size(_private_key.get()) - 11);
    Bytes encrypt_message(RSA_size(_private_key.get()));
    RSA_private_encrypt(message.size(), reinterpret_cast<const unsigned char*>(message.toArray()),
                        reinterpret_cast<unsigned char*>(encrypt_message.toArray()), _private_key.get(),
                        RSA_PKCS1_PADDING);
    return encrypt_message;
}


Bytes Rsa::public_encrypt(const Bytes& message) const
{
    ASSERT(message.size() <= RSA_size(_public_key.get()) - 42);
    Bytes encrypt_message(RSA_size(_public_key.get()));
    RSA_public_encrypt(message.size(), reinterpret_cast<const unsigned char*>(message.toArray()),
                       reinterpret_cast<unsigned char*>(encrypt_message.toArray()), _public_key.get(),
                       RSA_PKCS1_OAEP_PADDING);
    return encrypt_message;
}


Bytes Rsa::private_decrypt(const Bytes& encrypt_message) const
{
    ASSERT(encrypt_message.size() <= RSA_size(_private_key.get()));
    Bytes decrypt_message(RSA_size(_private_key.get()));
    auto message_size = RSA_private_decrypt(encrypt_message.size(), reinterpret_cast<const unsigned char*>(encrypt_message.toArray()),
                        reinterpret_cast<unsigned char*>(decrypt_message.toArray()), _private_key.get(),
                        RSA_PKCS1_OAEP_PADDING);
    return decrypt_message.takePart(0, message_size);
}


Bytes Rsa::public_decrypt(const Bytes& encrypt_message) const
{
    ASSERT(encrypt_message.size() <= RSA_size(_public_key.get()));
    Bytes decrypt_message(RSA_size(_public_key.get()));
    auto message_size = RSA_public_decrypt(encrypt_message.size(), reinterpret_cast<const unsigned char*>(encrypt_message.toArray()),
                       reinterpret_cast<unsigned char*>(decrypt_message.toArray()), _public_key.get(),
                       RSA_PKCS1_PADDING);
    return decrypt_message.takePart(0, message_size);
}


void Rsa::save(const std::filesystem::path& public_path, const std::filesystem::path& private_path) const
{

    std::unique_ptr<FILE> public_file(fopen(public_path.c_str(), "w"));
    std::unique_ptr<FILE> private_file(fopen(private_path.c_str(), "w"));
    PEM_write_RSAPublicKey(public_file.get(), _public_key.get());
    PEM_write_RSAPrivateKey(private_file.get(), _private_key.get(), NULL, NULL, 0, NULL, NULL);
}

} // namespace base