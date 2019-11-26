#include "asymmetric_crypto.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/bn.h>
#include <openssl/pem.h>

#include <memory>

namespace base
{

namespace rsa
{

    PublicKey::PublicKey(const base::Bytes& key_word)
        : _rsa_key(loadKey(key_word)), _encrypted_message_size(RSA_size(_rsa_key.get()))
    {}

    Bytes PublicKey::encrypt(const Bytes& message) const
    {
        if(message.size() > maxEncryptSize()) {
            RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
        }

        Bytes encrypted_message(encryptedMessageSize());
        if(!RSA_public_encrypt(message.size(), message.toArray(), encrypted_message.toArray(), _rsa_key.get(),
               RSA_PKCS1_OAEP_PADDING)) {
            RAISE_ERROR(CryptoError, "rsa encryption failed");
        }

        return encrypted_message;
    }

    Bytes PublicKey::decrypt(const Bytes& encrypted_message) const
    {
        if(encrypted_message.size() != encryptedMessageSize()) {
            RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
        }

        Bytes decrypted_message(encryptedMessageSize());
        auto message_size = RSA_public_decrypt(encrypted_message.size(), encrypted_message.toArray(),
            decrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_PADDING);
        if(message_size == -1) {
            RAISE_ERROR(CryptoError, "rsa decryption failed");
        }

        return decrypted_message.takePart(0, message_size);
    }

    std::size_t PublicKey::maxEncryptSize() const noexcept
    {
        return encryptedMessageSize() - ASYMMETRIC_DIFFERENCE;
    }

    Bytes PublicKey::toBytes() const
    {
        std::unique_ptr<BIO, decltype(&::BIO_free)> public_bio(BIO_new(BIO_s_mem()), ::BIO_free);

        if(!PEM_write_bio_RSAPublicKey(public_bio.get(), _rsa_key.get())) {
            RAISE_ERROR(CryptoError, "failed to write public RSA key to big num");
        }

        Bytes public_key_bytes(BIO_pending(public_bio.get()));
        if(!BIO_read(public_bio.get(), public_key_bytes.toArray(), public_key_bytes.size())) {
            RAISE_ERROR(CryptoError, "failed to write big num with public RSA to bytes");
        }

        return public_key_bytes;
    }

    std::size_t PublicKey::encryptedMessageSize() const noexcept
    {
        return _encrypted_message_size;
    }

    std::unique_ptr<RSA, decltype(&::RSA_free)> PublicKey::loadKey(const Bytes& key_word)
    {
        std::unique_ptr<BIO, decltype(&::BIO_free)> bio(
            BIO_new_mem_buf(key_word.toArray(), key_word.size()), ::BIO_free);
        RSA* rsa_key = nullptr;
        if(!PEM_read_bio_RSAPublicKey(bio.get(), &rsa_key, NULL, NULL)) {
            RAISE_ERROR(CryptoError, "Fail to read public RSA key");
        }
        return std::unique_ptr<RSA, decltype(&::RSA_free)>(rsa_key, ::RSA_free);
    }

    PrivateKey::PrivateKey(const base::Bytes& key_word)
        : _rsa_key(loadKey(key_word)), _encrypted_message_size(RSA_size(_rsa_key.get()))
    {}

    Bytes PrivateKey::encrypt(const Bytes& message) const
    {
        if(message.size() > maxMessageSizeForEncrypt()) {
            RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
        }

        Bytes encrypted_message(encryptedMessageSize());
        if(!RSA_private_encrypt(
               message.size(), message.toArray(), encrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_PADDING)) {
            RAISE_ERROR(CryptoError, "rsa encryption failed");
        }

        return encrypted_message;
    }

    Bytes PrivateKey::decrypt(const Bytes& encrypted_message) const
    {
        if(encrypted_message.size() != encryptedMessageSize()) {
            RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
        }

        Bytes decrypt_message(encryptedMessageSize());
        auto message_size = RSA_private_decrypt(encrypted_message.size(), encrypted_message.toArray(),
            decrypt_message.toArray(), _rsa_key.get(), RSA_PKCS1_OAEP_PADDING);
        if(message_size == -1) {
            RAISE_ERROR(CryptoError, "rsa decryption failed");
        }
        return decrypt_message.takePart(0, message_size);
    }

    std::size_t PrivateKey::maxMessageSizeForEncrypt() const noexcept
    {
        return encryptedMessageSize() - ASYMMETRIC_DIFFERENCE;
    }

    Bytes PrivateKey::toBytes() const
    {
        std::unique_ptr<BIO, decltype(&::BIO_free)> private_bio(BIO_new(BIO_s_mem()), ::BIO_free);

        if(!PEM_write_bio_RSAPrivateKey(private_bio.get(), _rsa_key.get(), NULL, NULL, 0, 0, NULL)) {
            RAISE_ERROR(CryptoError, "failed to write private RSA key to big num");
        }

        Bytes private_key_bytes(BIO_pending(private_bio.get()));
        if(!BIO_read(private_bio.get(), private_key_bytes.toArray(), BIO_pending(private_bio.get()))) {
            RAISE_ERROR(CryptoError, "failed to write big num with private RSA to bytes");
        }

        return private_key_bytes;
    }

    std::size_t PrivateKey::encryptedMessageSize() const noexcept
    {
        return _encrypted_message_size;
    }

    std::unique_ptr<RSA, decltype(&::RSA_free)> PrivateKey::loadKey(const Bytes& key_word)
    {
        std::unique_ptr<BIO, decltype(&::BIO_free)> bio(
            BIO_new_mem_buf(key_word.toArray(), key_word.size()), ::BIO_free);
        RSA* rsa_key = nullptr;
        if(!PEM_read_bio_RSAPrivateKey(bio.get(), &rsa_key, NULL, NULL)) {
            RAISE_ERROR(CryptoError, "Fail to read private RSA key");
        }
        return std::unique_ptr<RSA, decltype(&::RSA_free)>(rsa_key, ::RSA_free);
    }

    std::pair<PublicKey, PrivateKey> generateKeys(std::size_t keys_size)
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

} // namespace rsa

}; // namespace base
