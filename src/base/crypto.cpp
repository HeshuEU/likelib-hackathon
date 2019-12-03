#include "crypto.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>

namespace
{
base::Bytes read_all_file(const std::filesystem::path& path)
{
    if(!std::filesystem::exists(path)) {
        throw std::runtime_error("the file with this path does not exist");
    }
    std::ifstream file(path);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
    return base::Bytes(buffer);
}

base::Bytes generate_bytes(std::size_t size)
{
    std::vector<base::Byte> data(size);
    RAND_bytes(data.data(), static_cast<int>(size));
    return base::Bytes(data);
}

} // namespace


namespace base
{

RsaPublicKey::RsaPublicKey(const base::Bytes& key_word)
    : _rsa_key(loadKey(key_word)), _encrypted_message_size(RSA_size(_rsa_key.get()))
{}


RsaPublicKey::RsaPublicKey(const std::filesystem::path& path)
    : _rsa_key(loadKey(read_all_file(path))), _encrypted_message_size(RSA_size(_rsa_key.get()))
{}


Bytes RsaPublicKey::encrypt(const Bytes& message) const
{
    if(message.size() > maxEncryptSize()) {
        RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
    }

    Bytes encrypted_message(encryptedMessageSize());
    if(!RSA_public_encrypt(
           message.size(), message.toArray(), encrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_OAEP_PADDING)) {
        RAISE_ERROR(CryptoError, "rsa encryption failed");
    }

    return encrypted_message;
}


Bytes RsaPublicKey::encryptWithAes(const Bytes& message) const
{
    if(maxEncryptSize() < 256) {
        RAISE_ERROR(CryptoError, "small Rsa key size for RsaAes encryption");
    }
    base::AesKey symmetric_key(base::AesKey::KeyType::K256BIT);
    auto encrypted_message = symmetric_key.encrypt(message);
    auto serialized_symmetric_key = symmetric_key.toBytes();
    auto encrypted_serialized_symmetric_key = encrypt(serialized_symmetric_key);

    Bytes encrypted_serialized_key_size(sizeof(std::uint_least32_t));
    std::uint_least32_t key_size = encrypted_serialized_symmetric_key.size();
    std::memcpy(encrypted_serialized_key_size.toArray(), &key_size, encrypted_serialized_key_size.size());

    return encrypted_serialized_key_size.append(encrypted_serialized_symmetric_key).append(encrypted_message);
}


Bytes RsaPublicKey::decrypt(const Bytes& encrypted_message) const
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


std::size_t RsaPublicKey::maxEncryptSize() const noexcept
{
    return encryptedMessageSize() - ASYMMETRIC_DIFFERENCE;
}


void RsaPublicKey::save(const std::filesystem::path& path) const
{
    std::ofstream file(path);
    file << toBytes().toString();
}


Bytes RsaPublicKey::toBytes() const
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


std::size_t RsaPublicKey::encryptedMessageSize() const noexcept
{
    return _encrypted_message_size;
}


std::unique_ptr<RSA, decltype(&::RSA_free)> RsaPublicKey::loadKey(const Bytes& key_word)
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new_mem_buf(key_word.toArray(), key_word.size()), ::BIO_free);
    RSA* rsa_key = nullptr;
    if(!PEM_read_bio_RSAPublicKey(bio.get(), &rsa_key, nullptr, nullptr)) {
        RAISE_ERROR(CryptoError, "Fail to read public RSA key");
    }
    return std::unique_ptr<RSA, decltype(&::RSA_free)>(rsa_key, ::RSA_free);
}


RsaPrivateKey::RsaPrivateKey(const base::Bytes& key_word)
    : _rsa_key(loadKey(key_word)), _encrypted_message_size(RSA_size(_rsa_key.get()))
{}


RsaPrivateKey::RsaPrivateKey(const std::filesystem::path& path)
    : _rsa_key(loadKey(read_all_file(path))), _encrypted_message_size(RSA_size(_rsa_key.get()))
{}


Bytes RsaPrivateKey::encrypt(const Bytes& message) const
{
    if(message.size() > maxEncryptSize()) {
        RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
    }

    Bytes encrypted_message(encryptedMessageSize());
    if(!RSA_private_encrypt(
           message.size(), message.toArray(), encrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_PADDING)) {
        RAISE_ERROR(CryptoError, "rsa encryption failed");
    }

    return encrypted_message;
}


Bytes RsaPrivateKey::decrypt(const Bytes& encrypted_message) const
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


Bytes RsaPrivateKey::decryptWithAes(const Bytes& message) const
{
    Bytes encrypted_serialized_key_size = message.takePart(0, sizeof(std::uint_least32_t));
    std::uint_least32_t key_size = 0;
    std::memcpy(&key_size, encrypted_serialized_key_size.toArray(), encrypted_serialized_key_size.size());

    auto encrypted_serialized_symmetric_key =
        message.takePart(sizeof(std::uint_least32_t), key_size + sizeof(std::uint_least32_t));
    auto encrypted_message = message.takePart(key_size + sizeof(std::uint_least32_t), message.size());

    auto serialized_symmetric_key = decrypt(encrypted_serialized_symmetric_key);
    AesKey symmetric_key(serialized_symmetric_key);

    return symmetric_key.decrypt(encrypted_message);
}


std::size_t RsaPrivateKey::maxEncryptSize() const noexcept
{
    return encryptedMessageSize() - ASYMMETRIC_DIFFERENCE;
}


void RsaPrivateKey::save(const std::filesystem::path& path) const
{
    std::ofstream file(path);
    file << toBytes().toString();
}


Bytes RsaPrivateKey::toBytes() const
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> private_bio(BIO_new(BIO_s_mem()), ::BIO_free);

    if(!PEM_write_bio_RSAPrivateKey(private_bio.get(), _rsa_key.get(), nullptr, nullptr, 0, nullptr, nullptr)) {
        RAISE_ERROR(CryptoError, "failed to write private RSA key to big num");
    }

    Bytes private_key_bytes(BIO_pending(private_bio.get()));
    if(!BIO_read(private_bio.get(), private_key_bytes.toArray(), BIO_pending(private_bio.get()))) {
        RAISE_ERROR(CryptoError, "failed to write big num with private RSA to bytes");
    }

    return private_key_bytes;
}


std::size_t RsaPrivateKey::encryptedMessageSize() const noexcept
{
    return _encrypted_message_size;
}


std::unique_ptr<RSA, decltype(&::RSA_free)> RsaPrivateKey::loadKey(const Bytes& key_word)
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new_mem_buf(key_word.toArray(), key_word.size()), ::BIO_free);
    RSA* rsa_key = nullptr;
    if(!PEM_read_bio_RSAPrivateKey(bio.get(), &rsa_key, nullptr, nullptr)) {
        RAISE_ERROR(CryptoError, "Fail to read private RSA key");
    }
    return std::unique_ptr<RSA, decltype(&::RSA_free)>(rsa_key, ::RSA_free);
}


std::pair<RsaPublicKey, RsaPrivateKey> generateKeys(std::size_t keys_size)
{
    // create big number for random generation
    std::unique_ptr<BIGNUM, decltype(&::BN_free)> bn(BN_new(), ::BN_free);
    if(!BN_set_word(bn.get(), RSA_F4)) {
        RAISE_ERROR(Error, "Fail to create big number for RSA generation");
    }
    // create rsa and fill by created big number
    std::unique_ptr<RSA, decltype(&::RSA_free)> rsa(RSA_new(), ::RSA_free);
    if(!RSA_generate_key_ex(rsa.get(), keys_size, bn.get(), nullptr)) {
        RAISE_ERROR(Error, "Fail to generate RSA key");
    }
    // ==================
    // create bio for public key
    std::unique_ptr<BIO, decltype(&::BIO_free)> public_bio(BIO_new(BIO_s_mem()), ::BIO_free);

    // get public key spec
    std::unique_ptr<RSA, decltype(&::RSA_free)> public_rsa_key(RSAPublicKey_dup(rsa.get()), ::RSA_free);


    // fill bio by public key spec
    if(!PEM_write_bio_RSAPublicKey(public_bio.get(), public_rsa_key.get())) {
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
    std::unique_ptr<RSA, decltype(&::RSA_free)> private_rsa_key(RSAPrivateKey_dup(rsa.get()), ::RSA_free);

    // fill bio by private key spec
    if(!PEM_write_bio_RSAPrivateKey(private_bio.get(), private_rsa_key.get(), nullptr, nullptr, 0, nullptr, nullptr)) {
        RAISE_ERROR(Error, "Fail to generate private RSA key");
    }

    // write rsa data
    Bytes private_key_bytes(BIO_pending(private_bio.get()));

    // check errors in generation
    if(!BIO_read(private_bio.get(), private_key_bytes.toArray(), BIO_pending(private_bio.get()))) {
        RAISE_ERROR(Error, "Fail to check private RSA key");
    }
    // =============

    return std::pair<RsaPublicKey, RsaPrivateKey>(public_key_bytes, private_key_bytes);
}


AesKey::AesKey() : _type(KeyType::K256BIT), _key(generateKey(KeyType::K256BIT)), _iv(generateIv(KeyType::K256BIT))
{}


AesKey::AesKey(KeyType type) : _type(type), _key(generateKey(type)), _iv(generateIv(type))
{}


AesKey::AesKey(const Bytes& bytes_key)
{
    switch(bytes_key.size()) {
        case _aes_256_size:
            _type = KeyType::K256BIT;
            _key = bytes_key.takePart(0, 16 * 2);
            _iv = bytes_key.takePart(16 * 2, bytes_key.size());
            break;
        case _aes_128_size:
            _type = KeyType::K128BIT;
            _key = bytes_key.takePart(0, 8 * 2);
            _iv = bytes_key.takePart(8 * 2, bytes_key.size());
            break;
        default:
            RAISE_ERROR(InvalidArgument, "bytes_key are not valid. They must be obtained by Key::toBytes");
    }
}


Bytes AesKey::toBytes() const
{
    return Bytes(_key.toString() + _iv.toString()); // concatenate size = iv.size() * 3
}


Bytes AesKey::encrypt(const Bytes& data) const
{
    switch(_type) {
        case KeyType::K256BIT:
            return encrypt256Aes(data);
        case KeyType::K128BIT:
            return encrypt128Aes(data);
        default:
            RAISE_ERROR(CryptoError, "Unexpected key type");
    }
}


Bytes AesKey::decrypt(const Bytes& data) const
{
    switch(_type) {
        case KeyType::K256BIT:
            return decrypt256Aes(data);
        case KeyType::K128BIT:
            return decrypt128Aes(data);
        default:
            RAISE_ERROR(CryptoError, "Unexpected key type");
    }
}


Bytes AesKey::generateKey(KeyType type)
{
    switch(type) {
        case KeyType::K256BIT:
            return generate_bytes(16 * 2); // 32(bytes) * 8(bit in byte) = 256(bit)
        case KeyType::K128BIT:
            return generate_bytes(8 * 2); // 16(bytes) * 8(bit in byte) = 128(bit)
        default:
            RAISE_ERROR(CryptoError, "Unexpected key type");
    }
}


Bytes AesKey::generateIv(KeyType type)
{
    switch(type) {
        case KeyType::K256BIT:
            return generate_bytes(16); // 16(bytes) * 8(bit in byte) = 128(bit)
        case KeyType::K128BIT:
            return generate_bytes(8); // 8(bytes) * 8(bit in byte) = 64(bit)
        default:
            RAISE_ERROR(CryptoError, "Unexpected key type");
    }
}


Bytes AesKey::encrypt256Aes(const Bytes& data) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if(1 != EVP_EncryptInit_ex(context.get(), EVP_aes_256_cbc(), nullptr, _key.toArray(), _iv.toArray())) {
        RAISE_ERROR(CryptoError, "failed to initialize context");
    }

    Bytes output_data(data.size() * 2);

    int current_data_len = 0;
    if(1 != EVP_EncryptUpdate(context.get(), output_data.toArray(), &current_data_len, data.toArray(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to encrypt message");
    }
    int encrypted_message_len_in_buffer = current_data_len;

    if(1 != EVP_EncryptFinal_ex(context.get(), output_data.toArray() + current_data_len, &current_data_len)) {
        RAISE_ERROR(CryptoError, "unable to finalize encrypt");
    }
    encrypted_message_len_in_buffer += current_data_len;

    return output_data.takePart(0, encrypted_message_len_in_buffer);
}


base::Bytes AesKey::decrypt256Aes(const base::Bytes& data) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if(1 != EVP_DecryptInit_ex(context.get(), EVP_aes_256_cbc(), nullptr, _key.toArray(), _iv.toArray())) {
        RAISE_ERROR(CryptoError, "failed to initialize context");
    }

    Bytes output_data(data.size() * 2);

    int current_data_len = 0;
    if(1 != EVP_DecryptUpdate(context.get(), output_data.toArray(), &current_data_len, data.toArray(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to decrypt message");
    }
    int decrypted_message_len_in_buffer = current_data_len;

    if(1 != EVP_DecryptFinal_ex(context.get(), output_data.toArray() + current_data_len, &current_data_len)) {
        RAISE_ERROR(CryptoError, "unable to finalize decrypt");
    }
    decrypted_message_len_in_buffer += current_data_len;

    return output_data.takePart(0, decrypted_message_len_in_buffer);
}


base::Bytes AesKey::encrypt128Aes(const base::Bytes& data) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if(1 != EVP_EncryptInit_ex(context.get(), EVP_aes_128_cbc(), nullptr, _key.toArray(), _iv.toArray())) {
        RAISE_ERROR(CryptoError, "failed to initialize context");
    }

    Bytes output_data(data.size() * 2);

    int current_data_len = 0;
    if(1 != EVP_EncryptUpdate(context.get(), output_data.toArray(), &current_data_len, data.toArray(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to encrypt message");
    }
    int encrypted_message_len_in_buffer = current_data_len;

    if(1 != EVP_EncryptFinal_ex(context.get(), output_data.toArray() + current_data_len, &current_data_len)) {
        RAISE_ERROR(CryptoError, "unable to finalize encrypt");
    }
    encrypted_message_len_in_buffer += current_data_len;

    return output_data.takePart(0, encrypted_message_len_in_buffer);
}


base::Bytes AesKey::decrypt128Aes(const base::Bytes& data) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if(1 != EVP_DecryptInit_ex(context.get(), EVP_aes_128_cbc(), nullptr, _key.toArray(), _iv.toArray())) {
        RAISE_ERROR(CryptoError, "failed to initialize context");
    }

    Bytes output_data(data.size() * 2);

    int current_data_len = 0;
    if(1 != EVP_DecryptUpdate(context.get(), output_data.toArray(), &current_data_len, data.toArray(), data.size()))
        RAISE_ERROR(CryptoError, "failed to decrypt message");
    int decrypted_message_len_in_buffer = current_data_len;

    if(1 != EVP_DecryptFinal_ex(context.get(), output_data.toArray() + current_data_len, &current_data_len))
        RAISE_ERROR(CryptoError, "unable to finalize decrypt");
    decrypted_message_len_in_buffer += current_data_len;

    return output_data.takePart(0, decrypted_message_len_in_buffer);
}


} // namespace base