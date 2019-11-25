#include "asymmetric_crypto.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/bn.h>
#include <openssl/pem.h>

#include <memory>


namespace
{

RSA* loadPublicRsaKey(const base::Bytes& key)
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new_mem_buf(key.toArray(), key.size()), ::BIO_free);

    RSA* rsa_key = nullptr;
    if(!PEM_read_bio_RSAPublicKey(bio.get(), &rsa_key, NULL, NULL)) {
        RAISE_ERROR(base::Error, "Fail to read public RSA key");
    }
    return rsa_key;
}

RSA* loadPrivateRsaKey(const base::Bytes& key)
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new_mem_buf(key.toArray(), key.size()), ::BIO_free);

    RSA* rsa_key = nullptr;
    if(!PEM_read_bio_RSAPrivateKey(bio.get(), &rsa_key, NULL, NULL)) {
        RAISE_ERROR(base::Error, "Fail to read private RSA key");
    }
    return rsa_key;
}

} // namespace

namespace base
{

PublicKey::PublicKey(const base::Bytes& key_word) : _public_key(key_word)
{
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(loadPublicRsaKey(_public_key), ::RSA_free);
    _encrypted_message_size = RSA_size(rsa_key.get());
}

Bytes PublicKey::encrypt(const Bytes& message) const
{
    // check for max message size
    if(message.size() > maxEncryptSize()) {
        RAISE_ERROR(InvalidArgument, "Large message size for RSA encryption");
    }

    // load rsa public key
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(loadPublicRsaKey(_public_key), ::RSA_free);

    // encrypt
    Bytes encrypted_message(encryptedMessageSize());
    if(!RSA_public_encrypt(
           message.size(), message.toArray(), encrypted_message.toArray(), rsa_key.get(), RSA_PKCS1_OAEP_PADDING)) {
        RAISE_ERROR(Error, "Error in RSA encryption");
    }

    return encrypted_message;
}

Bytes PublicKey::decrypt(const Bytes& encrypted_message) const
{
    if(encrypted_message.size() != encryptedMessageSize()) {
        RAISE_ERROR(InvalidArgument, "Large message size for RSA encryption");
    }
    // load rsa public key
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(loadPublicRsaKey(_public_key), ::RSA_free);

    // decrypt
    Bytes decrypted_message(encryptedMessageSize());
    auto message_size = RSA_public_decrypt(encrypted_message.size(), encrypted_message.toArray(),
        decrypted_message.toArray(), rsa_key.get(), RSA_PKCS1_PADDING);
    if(message_size == -1) {
        RAISE_ERROR(Error, "Error in RSA decryption");
    }

    return decrypted_message.takePart(0, message_size);
}

std::size_t PublicKey::encryptedMessageSize() const noexcept
{
    return _encrypted_message_size;
}

std::size_t PublicKey::maxEncryptSize() const noexcept
{
    return encryptedMessageSize() - ASYMMETRIC_DIFFERENCE;
}


Bytes PublicKey::toBytes() const noexcept
{
    return _public_key;
}


PrivateKey::PrivateKey(const base::Bytes& key_word) : _private_key(key_word)
{
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(loadPrivateRsaKey(_private_key), ::RSA_free);
    _encrypted_message_size = RSA_size(rsa_key.get());
}

Bytes PrivateKey::encrypt(const Bytes& message) const
{
    // check for max message size
    if(message.size() > maxMessageSizeForEncrypt()) {
        RAISE_ERROR(InvalidArgument, "Large message size for RSA encryption");
    }

    // load rsa private key
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(loadPrivateRsaKey(_private_key), ::RSA_free);

    // encrypt
    Bytes encrypted_message(encryptedMessageSize());
    if(!RSA_private_encrypt(
           message.size(), message.toArray(), encrypted_message.toArray(), rsa_key.get(), RSA_PKCS1_PADDING)) {
        RAISE_ERROR(Error, "Error in RSA encryption");
    }

    return encrypted_message;
}

Bytes PrivateKey::decrypt(const Bytes& encrypted_message) const
{
    // check for max message size
    if(encrypted_message.size() != encryptedMessageSize()) {
        RAISE_ERROR(InvalidArgument, "Large message size for RSA decryption");
    }

    // load rsa private key
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(loadPrivateRsaKey(_private_key), ::RSA_free);

    // decrypting
    Bytes decrypt_message(encryptedMessageSize());
    auto message_size = RSA_private_decrypt(encrypted_message.size(), encrypted_message.toArray(),
        decrypt_message.toArray(), rsa_key.get(), RSA_PKCS1_OAEP_PADDING);
    if(message_size == -1) {
        RAISE_ERROR(Error, "Error in RSA decryption");
    }
    return decrypt_message.takePart(0, message_size);
}

std::size_t PrivateKey::encryptedMessageSize() const noexcept
{
    return _encrypted_message_size;
}

std::size_t PrivateKey::maxMessageSizeForEncrypt() const noexcept
{
    return encryptedMessageSize() - ASYMMETRIC_DIFFERENCE;
}

Bytes PrivateKey::toBytes() const noexcept
{
    return _private_key;
}

std::pair<PublicKey, PrivateKey> generateRsaKeys(std::size_t keys_size)
{
    // create big number for random generation
    std::unique_ptr<BIGNUM, decltype(&::BN_free)> bn(BN_new(), ::BN_free);
    if(!BN_set_word(bn.get(), RSA_F4)) {
        RAISE_ERROR(Error, "Fail to create big number for RSA generation");
    }
    // create rsa and fill by created big number
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa(RSA_new(), ::RSA_free);
    if(!RSA_generate_key_ex(rsa.get(), keys_size, bn.get(), NULL)) {
        RAISE_ERROR(Error, "Fail to generate RSA key");
    }
    // ==================
    // create bio for public key
    std::unique_ptr<BIO, decltype(&::BIO_free)> public_bio(BIO_new(BIO_s_mem()), ::BIO_free);

    // get public key spec
    auto public_rsa_key = RSAPublicKey_dup(rsa.get());

    // fill bio by public key spec
    if(!PEM_write_bio_RSAPublicKey(public_bio.get(), public_rsa_key)) {
        RAISE_ERROR(Error, "Fail to generate public RSA key");
    }

    // write rsa data
    Bytes public_key_bytes(BIO_pending(public_bio.get()));

    // check errors in generation
    if(!BIO_read(public_bio.get(), public_key_bytes.toArray(), public_key_bytes.size())) {
        RAISE_ERROR(Error, "Fail to check public RSA key");
    }
    // =============

    // create bio for private key
    std::unique_ptr<BIO, decltype(&::BIO_free)> private_bio(BIO_new(BIO_s_mem()), ::BIO_free);

    // get private key spec
    auto private_rsa_key = RSAPrivateKey_dup(rsa.get());

    // fill bio by private key spec
    if(!PEM_write_bio_RSAPrivateKey(private_bio.get(), private_rsa_key, NULL, NULL, 0, 0, NULL)) {
        RAISE_ERROR(Error, "Fail to generate private RSA key");
    }

    // write rsa data
    Bytes private_key_bytes(BIO_pending(private_bio.get()));

    // check errors in generation
    if(!BIO_read(private_bio.get(), private_key_bytes.toArray(), BIO_pending(private_bio.get()))) {
        RAISE_ERROR(Error, "Fail to check private RSA key");
    }
    // =============

    return std::pair<PublicKey, PrivateKey>(public_key_bytes, private_key_bytes);
}


}; // namespace base
