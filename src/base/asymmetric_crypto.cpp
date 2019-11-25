#include "asymmetric_crypto.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/bn.h>
#include <openssl/pem.h>

#include <memory>

namespace base
{

PrivateKey::PrivateKey(const base::Bytes& key) : _private_key(key)
{}


PublicKey::PublicKey(const base::Bytes& key) : _public_key(key)
{}

std::pair<PublicKey, PrivateKey> generate(std::size_t keys_size)
{

    std::unique_ptr<BIGNUM, decltype(&::BN_free)> bn(BN_new(), ::BN_free);
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa(RSA_new(), ::RSA_free);
    if(!BN_set_word(bn.get(), RSA_F4)) {
        RAISE_ERROR(Error, "Fail to create big number for RSA generation");
    }
    if(!RSA_generate_key_ex(rsa.get(), keys_size, bn.get(), NULL)) {
        RAISE_ERROR(Error, "Fail to generate RSA key");
    }

    Bytes public_key_bytes;
    std::size_t public_key_size = 0;
    {
        RSA* public_key = RSAPublicKey_dup(rsa.get());
        std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new(BIO_s_mem()), ::BIO_free);
        if(!PEM_write_bio_RSAPublicKey(bio.get(), public_key)) {
            RAISE_ERROR(Error, "Fail to generate RSA key");
        }
        public_key_bytes = Bytes(BIO_pending(bio.get()));
        public_key_size = RSA_size(public_key);
        ASSERT(BIO_read(bio.get(), public_key_bytes.toArray(), public_key_bytes.size()));
    }

    Bytes private_key_bytes;
    std::size_t private_key_size = 0;
    {
        RSA* private_key = RSAPrivateKey_dup(rsa.get());
        std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new(BIO_s_mem()), ::BIO_free);
        ASSERT(PEM_write_bio_RSAPrivateKey(bio.get(), private_key, NULL, NULL, 0, 0, NULL)); // check errors
        private_key_bytes = Bytes(BIO_pending(bio.get()));
        private_key_size = RSA_size(private_key);
        ASSERT(BIO_read(bio.get(), private_key_bytes.toArray(), BIO_pending(bio.get()))); // check errors
    }

    return std::pair<PublicKey, PrivateKey>(public_key_bytes, private_key_bytes);
}


}; // namespace base
