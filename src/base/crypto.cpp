#include "crypto.hpp"

#include "base/assert.hpp"

#include <openssl/bn.h>

namespace base
{

Rsa::Rsa() : _private_key(RSA_new(), ::RSA_free), _public_key(RSA_new(), ::RSA_free)
{
    std::unique_ptr<BIGNUM, decltype(&::BN_free)> bn(BN_new(), ::BN_free);
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa(RSA_new(), ::RSA_free);
    BN_set_word(bn.get(), RSA_F4);
    RSA_generate_key_ex(rsa.get(), 2048, bn.get(), NULL);

    _private_key = std::unique_ptr<RSA, decltype(&::RSA_free)>(RSAPrivateKey_dup(rsa.get()), ::RSA_free);
    _public_key = std::unique_ptr<RSA, decltype(&::RSA_free)>(RSAPublicKey_dup(rsa.get()), ::RSA_free);
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

std::string Rsa::private_encrypt(const std::string& message) const
{
    ASSERT(message.length() - 42 <= RSA_size(_private_key.get()));
    char* encrypt_message = new char[RSA_size(_private_key.get()) + 1];  //can't use string because c_str() return const char*
    encrypt_message[RSA_size(_private_key.get())] = '\0';
    RSA_private_encrypt(message.length(), reinterpret_cast<const unsigned char*>(message.c_str()),
                       reinterpret_cast<unsigned char*>(encrypt_message), _private_key.get(), RSA_PKCS1_OAEP_PADDING);
    return std::string(encrypt_message);
}

std::string Rsa::public_encrypt(const std::string& message) const
{
    ASSERT(message.length() - 42 <= RSA_size(_public_key.get()));
    char* encrypt_message = new char[RSA_size(_public_key.get()) + 1]; 
    encrypt_message[RSA_size(_public_key.get())] = '\0';
    RSA_public_encrypt(message.length(), reinterpret_cast<const unsigned char*>(message.c_str()),
                       reinterpret_cast<unsigned char*>(encrypt_message), _public_key.get(), RSA_PKCS1_OAEP_PADDING);
    return std::string(encrypt_message);
}

std::string Rsa::private_decrypt(const std::string& encrypt_message) const
{
    ASSERT(encrypt_message.length() <= RSA_size(_private_key.get()));
    char* decrypt_message = new char[RSA_size(_private_key.get()) + 1];
    decrypt_message[RSA_size(_private_key.get())] = '\0';
    RSA_private_decrypt(encrypt_message.length(), reinterpret_cast<const unsigned char*>(encrypt_message.c_str()),
                       reinterpret_cast<unsigned char*>(decrypt_message), _private_key.get(), RSA_PKCS1_OAEP_PADDING);
    return std::string(decrypt_message);
}

std::string Rsa::public_decrypt(const std::string& encrypt_message) const
{
    ASSERT(encrypt_message.length() <= RSA_size(_public_key.get()));
    char* decrypt_message = new char[RSA_size(_public_key.get()) + 1];
    decrypt_message[RSA_size(_public_key.get())] = '\0';
    RSA_public_decrypt(encrypt_message.length(), reinterpret_cast<const unsigned char*>(encrypt_message.c_str()),
                       reinterpret_cast<unsigned char*>(decrypt_message), _public_key.get(), RSA_PKCS1_OAEP_PADDING);
    return std::string(decrypt_message);
}

} // namespace base