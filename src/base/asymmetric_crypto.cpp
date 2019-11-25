#include "asymmetric_crypto.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/bn.h>
#include <openssl/pem.h>

#include <memory>

namespace base
{

PrivateKey::PrivateKey(const base::Bytes& key_word, std::size_t key_size) : _private_key(key_word), _key_size(key_size)
{}

PublicKey::PublicKey(const base::Bytes& key_word, std::size_t key_size) : _public_key(key_word), _key_size(key_size)
{}

std::pair<PublicKey, PrivateKey> generate(std::size_t keys_size)
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
    std::size_t public_key_size = RSA_size(public_rsa_key);

    // check errors in generation
    if(!BIO_read(public_bio.get(), public_key_bytes.toArray(), public_key_bytes.size())) {
        RAISE_ERROR(Error, "Fail to check public RSA key");
    }

    PublicKey public_key(public_key_bytes, public_key_size);
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
    std::size_t private_key_size = RSA_size(private_rsa_key);

    // check errors in generation
    if(!BIO_read(private_bio.get(), private_key_bytes.toArray(), BIO_pending(private_bio.get()))) {
        RAISE_ERROR(Error, "Fail to check private RSA key");
    }
    PrivateKey privateKey(private_key_bytes, private_key_size);
    // =============
    return std::pair<PublicKey, PrivateKey>(public_key, privateKey);
}


}; // namespace base
