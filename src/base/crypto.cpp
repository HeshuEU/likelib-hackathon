#include "crypto.hpp"

#include "base/assert.hpp"

#include <openssl/bn.h>
#include <openssl/pem.h>

namespace base
{

Rsa::Rsa(const size_t count_bites)
{
    std::shared_ptr<BIGNUM> bn(BN_new(), ::BN_free);
    std::shared_ptr<RSA> rsa(RSA_new(), ::RSA_free);
    BN_set_word(bn.get(), RSA_F4);
    RSA_generate_key_ex(rsa.get(), count_bites, bn.get(), NULL);
    _private_key = std::shared_ptr<RSA>(RSAPrivateKey_dup(rsa.get()), ::RSA_free);
    _public_key = std::shared_ptr<RSA>(RSAPublicKey_dup(rsa.get()), ::RSA_free);
}


Rsa::Rsa(const std::shared_ptr<RSA>& public_key) : _private_key(nullptr), _public_key(public_key)
{}


Rsa::Rsa(const std::filesystem::path& public_path, const std::filesystem::path& private_path)
{
    if(!std::filesystem::exists(public_path)) {
        throw std::runtime_error("Public_file_not_exist");
    }
    if(!std::filesystem::exists(private_path)) {
        throw std::runtime_error("Private_file_not_exist");
    }
    FILE* public_file = fopen(public_path.c_str(), "rb");
    FILE* private_file = fopen(private_path.c_str(), "rb");
    RSA* private_temp = RSA_new();
    RSA* public_temp = RSA_new();
    PEM_read_RSAPrivateKey(private_file, &private_temp, NULL, NULL);
    PEM_read_RSAPublicKey(public_file, &public_temp, NULL, NULL);
    _private_key = std::shared_ptr<RSA>(private_temp, ::RSA_free);
    _public_key = std::shared_ptr<RSA>(public_temp, ::RSA_free);
    fclose(public_file);
    fclose(private_file);
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


Bytes Rsa::privateEncrypt(const Bytes& message) const
{
    ASSERT(message.size() <= maxPrivateEncryptSize());
    Bytes encrypt_message(RSA_size(_private_key.get()));
    RSA_private_encrypt(message.size(), reinterpret_cast<const unsigned char*>(message.toArray()),
                        reinterpret_cast<unsigned char*>(encrypt_message.toArray()), _private_key.get(),
                        RSA_PKCS1_PADDING);
    return encrypt_message;
}


Bytes Rsa::publicEncrypt(const Bytes& message) const
{
    ASSERT(message.size() <= maxPublicEncryptSize());
    Bytes encrypt_message(RSA_size(_public_key.get()));
    RSA_public_encrypt(message.size(), reinterpret_cast<const unsigned char*>(message.toArray()),
                       reinterpret_cast<unsigned char*>(encrypt_message.toArray()), _public_key.get(),
                       RSA_PKCS1_OAEP_PADDING);
    return encrypt_message;
}


Bytes Rsa::privateDecrypt(const Bytes& encrypt_message) const
{
    ASSERT(encrypt_message.size() <= size());
    Bytes decrypt_message(RSA_size(_private_key.get()));
    auto message_size = RSA_private_decrypt(
        encrypt_message.size(), reinterpret_cast<const unsigned char*>(encrypt_message.toArray()),
        reinterpret_cast<unsigned char*>(decrypt_message.toArray()), _private_key.get(), RSA_PKCS1_OAEP_PADDING);
    return decrypt_message.takePart(0, message_size);
}


Bytes Rsa::publicDecrypt(const Bytes& encrypt_message) const
{
    ASSERT(encrypt_message.size() <= size());
    Bytes decrypt_message(RSA_size(_public_key.get()));
    auto message_size = RSA_public_decrypt(
        encrypt_message.size(), reinterpret_cast<const unsigned char*>(encrypt_message.toArray()),
        reinterpret_cast<unsigned char*>(decrypt_message.toArray()), _public_key.get(), RSA_PKCS1_PADDING);
    return decrypt_message.takePart(0, message_size);
}


void Rsa::save(const std::filesystem::path& public_path, const std::filesystem::path& private_path) const
{
    FILE* public_file = fopen(public_path.c_str(), "w");
    PEM_write_RSAPublicKey(public_file, _public_key.get());
    fclose(public_file);

    if(private_path.string() != std::filesystem::path("-1")) {
        FILE* private_file = fopen(private_path.c_str(), "w");
        PEM_write_RSAPrivateKey(private_file, _private_key.get(), NULL, NULL, 0, NULL, NULL);
        fclose(private_file);
    }
}


size_t Rsa::size() const
{
    return RSA_size(_public_key.get());
}


size_t Rsa::maxPrivateEncryptSize() const
{
    return size() - 11;
}


size_t Rsa::maxPublicEncryptSize() const
{
    return size() - 42;
}

std::shared_ptr<RSA> Rsa::getPublicKey() const
{
    return _public_key;
}

} // namespace base