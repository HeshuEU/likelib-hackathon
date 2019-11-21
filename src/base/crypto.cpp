#include "crypto.hpp"

#include "base/assert.hpp"

#include <openssl/bn.h>
#include <openssl/pem.h>

#include <fstream>

namespace
{
std::string read_all_file(const std::filesystem::path& path)
{
    std::ifstream file(path);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
    return buffer;
}
} // namespace


namespace base
{

PublicRsaKey::PublicRsaKey(const std::filesystem::path& path)
{
    if(!std::filesystem::exists(path)) {
        throw std::runtime_error("Path for PublicRsaKey not exist");
    }
    _public_key = Bytes(read_all_file(path));
    _size = RSA_size(toRsaKey());
}


PublicRsaKey::PublicRsaKey(const Bytes& key) : _public_key(key)
{
    auto rsa_key = toRsaKey();
    _size = RSA_size(rsa_key);
}


Bytes PublicRsaKey::encrypt(const Bytes& message) const
{
    if(message.size() > maxEncryptSize()) {
        throw std::runtime_error("Large message size for RSA encryption");
    }
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(toRsaKey(), ::RSA_free);
    Bytes encrypted_message(size());
    if(!RSA_public_encrypt(
           message.size(), message.toArray(), encrypted_message.toArray(), rsa_key.get(), RSA_PKCS1_OAEP_PADDING)) {
        throw std::runtime_error("RSA public key encryption error");
    } // check errors
    return encrypted_message;
}


Bytes PublicRsaKey::encryptWithAes(const Bytes& message) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> rsa_ecnrypt_ctx(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    EVP_CIPHER_CTX_init(rsa_ecnrypt_ctx.get());
    Bytes iv(EVP_MAX_IV_LENGTH);
    Bytes ek(size());
    auto ek_temp = ek.toArray();
    auto evp_key = toEvpKey();
    int ek_len = 0;
    ASSERT(EVP_SealInit(rsa_ecnrypt_ctx.get(), EVP_aes_256_cbc(), &ek_temp, &ek_len, iv.toArray(), &evp_key,
        1)); // check errors
    Bytes encrypted_aes_message(message.size() + EVP_MAX_IV_LENGTH);
    int encrypted_block_len = 0;
    ASSERT(
        EVP_SealUpdate(rsa_ecnrypt_ctx.get(), encrypted_aes_message.toArray(), &encrypted_block_len, message.toArray(),
            message.size())); // check errors
    int encrypted_message_len = encrypted_block_len;
    ASSERT(EVP_SealFinal(rsa_ecnrypt_ctx.get(), encrypted_aes_message.toArray() + encrypted_block_len,
        &encrypted_block_len)); // check errors
    encrypted_message_len += encrypted_block_len;

    Bytes encrypted_message;
    std::string encrypted_message_len_str = std::to_string(encrypted_message_len);
    while(encrypted_message_len_str.size() < 9) {
        encrypted_message_len_str = '0' + encrypted_message_len_str;
    }
    std::string ek_len_str = std::to_string(ek_len);
    while(ek_len_str.size() < 9) {
        ek_len_str = '0' + ek_len_str;
    }
    encrypted_message.append(Bytes(encrypted_message_len_str));
    encrypted_message.append(Bytes(ek_len_str));
    encrypted_message.append(iv);
    encrypted_message.append(ek);
    encrypted_message.append(encrypted_aes_message);
    return encrypted_message;
}


Bytes PublicRsaKey::decrypt(const Bytes& encrypted_message) const
{
    ASSERT(encrypted_message.size() <= size());
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(toRsaKey(), ::RSA_free);
    Bytes decrypted_message(size());
    auto message_size = RSA_public_decrypt(encrypted_message.size(), encrypted_message.toArray(), // check errors
        decrypted_message.toArray(), rsa_key.get(), RSA_PKCS1_PADDING);
    ASSERT(message_size != -1);
    return decrypted_message.takePart(0, message_size);
}


std::size_t PublicRsaKey::size() const noexcept
{
    return _size;
}


std::size_t PublicRsaKey::maxEncryptSize() const noexcept
{
    return size() - 42;
}


Bytes PublicRsaKey::toBytes() const noexcept
{
    return _public_key;
}

void PublicRsaKey::save(const std::filesystem::path& path) const
{
    std::ofstream file(path);
    file << _public_key.toString();
}

PublicRsaKey::PublicRsaKey(RSA* key) : _public_key(RSA_size(key))
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new(BIO_s_mem()), ::BIO_free);
    ASSERT(PEM_write_bio_RSAPublicKey(bio.get(), key)); // check errors
    _public_key = Bytes(BIO_pending(bio.get()));
    _size = RSA_size(key);
    ASSERT(BIO_read(bio.get(), _public_key.toArray(), _public_key.size()));
}


PublicRsaKey::PublicRsaKey(EVP_PKEY* key) : _public_key(EVP_PKEY_size(key))
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new(BIO_s_mem()), ::BIO_free);
    ASSERT(PEM_write_bio_PUBKEY(bio.get(), key)); // check errors
    _public_key = Bytes(BIO_pending(bio.get()));
    _size = EVP_PKEY_size(key);
    ASSERT(BIO_read(bio.get(), _public_key.toArray(), _public_key.size()));
}


RSA* PublicRsaKey::toRsaKey() const
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(
        BIO_new_mem_buf(_public_key.toArray(), _public_key.size()), ::BIO_free);
    RSA* rsa_key = nullptr;
    ASSERT(PEM_read_bio_RSAPublicKey(bio.get(), &rsa_key, NULL, NULL)); // check errors
    return rsa_key;
}


EVP_PKEY* PublicRsaKey::toEvpKey() const
{
    EVP_PKEY* evp_rsa_key = EVP_PKEY_new();
    auto rsa_key = toRsaKey();
    ASSERT(EVP_PKEY_assign_RSA(evp_rsa_key, rsa_key)); // aspect on an existing element
    return evp_rsa_key;
}


PrivateRsaKey::PrivateRsaKey(const Bytes& key) : _private_key(key)
{
    auto rsa_key = toRsaKey();
    _size = _private_key.size();
}


PrivateRsaKey::PrivateRsaKey(const std::filesystem::path& path)
{
    if(!std::filesystem::exists(path)) {
        throw std::runtime_error("Path for PublicRsaKey not exist");
    }

    _private_key = Bytes(read_all_file(path));
    _size = RSA_size(toRsaKey());
}


Bytes PrivateRsaKey::encrypt(const Bytes& message) const
{
    if(message.size() > maxEncryptSize()) {
        throw std::runtime_error("Large message size for RSA encryption");
    }
    Bytes encrypted_message(size());
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(toRsaKey(), ::RSA_free);
    ASSERT(RSA_private_encrypt(message.size(), message.toArray(), encrypted_message.toArray(), rsa_key.get(),
        RSA_PKCS1_PADDING)); // check errors
    return encrypted_message;
}


Bytes PrivateRsaKey::decrypt(const Bytes& encrypted_message) const
{
    ASSERT(encrypted_message.size() <= size());
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa_key(toRsaKey(), ::RSA_free);
    Bytes decrypt_message(size());
    auto message_size = RSA_private_decrypt(encrypted_message.size(), encrypted_message.toArray(), // check errors
        decrypt_message.toArray(), rsa_key.get(), RSA_PKCS1_OAEP_PADDING);
    return decrypt_message.takePart(0, message_size);
}


Bytes PrivateRsaKey::decryptWithAes(const Bytes& encrypted_message) const
{
    ASSERT(encrypted_message.size() > EVP_MAX_IV_LENGTH + size());
    int encrypted_message_len = stoi(encrypted_message.takePart(0, 9).toString());
    int ek_len = stoi(encrypted_message.takePart(9, 18).toString());
    Bytes iv(encrypted_message.takePart(18, 18 + EVP_MAX_IV_LENGTH));
    Bytes ek(encrypted_message.takePart(18 + EVP_MAX_IV_LENGTH, 18 + EVP_MAX_IV_LENGTH + size()));
    Bytes encrypted_aes_message(encrypted_message.takePart(18 + EVP_MAX_IV_LENGTH + size(), encrypted_message.size()));
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> rsa_decrypt_ctx(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)> evp_key(toEvpKey(), ::EVP_PKEY_free);
    EVP_CIPHER_CTX_init(rsa_decrypt_ctx.get());
    ASSERT(EVP_OpenInit(
        rsa_decrypt_ctx.get(), EVP_aes_256_cbc(), ek.toArray(), ek_len, iv.toArray(), evp_key.get())); // check errors
    Bytes decrypted_message(encrypted_message_len + EVP_MAX_IV_LENGTH);
    int decrypted_block_len = 0;
    ASSERT(EVP_OpenUpdate(rsa_decrypt_ctx.get(), decrypted_message.toArray(), &decrypted_block_len,
        encrypted_aes_message.toArray(), encrypted_message_len)); // check errors
    ASSERT(EVP_OpenFinal(rsa_decrypt_ctx.get(), decrypted_message.toArray() + decrypted_block_len,
        &decrypted_block_len)); // check errors
    return decrypted_message.takePart(0, encrypted_aes_message.size() - EVP_MAX_IV_LENGTH);
}


std::size_t PrivateRsaKey::size() const noexcept
{
    return _size;
}


std::size_t PrivateRsaKey::maxEncryptSize() const noexcept
{
    return size() - 11;
}


Bytes PrivateRsaKey::toBytes() const noexcept
{
    return _private_key;
}


void PrivateRsaKey::save(const std::filesystem::path& path) const
{
    std::ofstream file(path);
    file << _private_key.toArray();
}


PrivateRsaKey::PrivateRsaKey(RSA* key)
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new(BIO_s_mem()), ::BIO_free);
    ASSERT(PEM_write_bio_RSAPrivateKey(bio.get(), key, NULL, NULL, 0, 0, NULL)); // check errors
    _private_key = Bytes(BIO_pending(bio.get()));
    _size = RSA_size(key);
    ASSERT(BIO_read(bio.get(), _private_key.toArray(), BIO_pending(bio.get()))); // check errors
}


PrivateRsaKey::PrivateRsaKey(EVP_PKEY* key)
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new(BIO_s_mem()), ::BIO_free);
    ASSERT(PEM_write_bio_PrivateKey(bio.get(), key, NULL, NULL, 0, 0, NULL)); // check errors
    _private_key = Bytes(BIO_pending(bio.get()));
    _size = EVP_PKEY_size(key);
    ASSERT(BIO_read(bio.get(), _private_key.toArray(), _private_key.size()));
}


RSA* PrivateRsaKey::toRsaKey() const
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(
        BIO_new_mem_buf(_private_key.toArray(), _private_key.size()), ::BIO_free);
    RSA* rsa_key = RSA_new();
    ASSERT(PEM_read_bio_RSAPrivateKey(bio.get(), &rsa_key, NULL, NULL)); // check errors
    return rsa_key;
}


EVP_PKEY* PrivateRsaKey::toEvpKey() const
{
    EVP_PKEY* evp_rsa_key = EVP_PKEY_new();
    auto rsa_key = toRsaKey();
    ASSERT(EVP_PKEY_assign_RSA(evp_rsa_key, rsa_key)); // aspect on an existing element
    return evp_rsa_key;
}


std::pair<PublicRsaKey, PrivateRsaKey> generate(const std::size_t keys_size)
{
    std::unique_ptr<BIGNUM, decltype(&::BN_free)> bn(BN_new(), ::BN_free);
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa(RSA_new(), ::RSA_free);
    ASSERT(BN_set_word(bn.get(), RSA_F4)); // check errors
    ASSERT(RSA_generate_key_ex(rsa.get(), keys_size, bn.get(), NULL)); // check errors
    return std::pair<PublicRsaKey, PrivateRsaKey>(RSAPublicKey_dup(rsa.get()), RSAPrivateKey_dup(rsa.get()));
}

} // namespace base