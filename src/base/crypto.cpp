#include "crypto.hpp"

#include "base/error.hpp"
#include "base/directory.hpp"
#include "base/log.hpp"
#include "base/hash.hpp"
#include "base/property_tree.hpp"

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

base::Bytes readAllFile(const std::filesystem::path& path)
{
    if(!std::filesystem::exists(path)) {
        RAISE_ERROR(base::InvalidArgument, "the file with this path does not exist");
    }
    std::ifstream file(path);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
    return base::Bytes::fromHex(buffer);
}


void writeFile(const std::filesystem::path& path, const base::Bytes& data)
{
    if(std::filesystem::exists(path)) {
        LOG_WARNING << "file[" << path << "] already exists";
    }
    else {
        auto dir = path.parent_path();
        if(!dir.empty()) {
            base::createIfNotExists(dir);
        }
    }

    std::ofstream file(path, std::ofstream::trunc);
    if(!file.is_open()) {
        RAISE_ERROR(base::InvalidArgument, "Failed to create file.");
    }
    file << data.toHex();
}


base::Bytes generate_bytes(std::size_t size)
{
    base::Bytes data(size);
    RAND_bytes(data.toArray(), static_cast<int>(size));
    return data;
}

} // namespace


namespace base
{

RsaPublicKey::RsaPublicKey(const base::Bytes& key_word)
    : _rsa_key(loadKey(key_word)), _encrypted_message_size(RSA_size(_rsa_key.get()))
{}


RsaPublicKey::RsaPublicKey(const RsaPublicKey& another)
    : _rsa_key(loadKey(another.toBytes())), _encrypted_message_size(another._encrypted_message_size)
{}


RsaPublicKey& RsaPublicKey::operator=(const RsaPublicKey& another)
{
    _rsa_key = loadKey(another.toBytes());
    _encrypted_message_size = another._encrypted_message_size;
    return *this;
}


Bytes RsaPublicKey::encrypt(const Bytes& message) const
{
    if(message.size() > maxEncryptSize()) {
        RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
    }

    Bytes encrypted_message(_encrypted_message_size);
    if(!RSA_public_encrypt(
           message.size(), message.toArray(), encrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_OAEP_PADDING)) {
        RAISE_ERROR(CryptoError, "rsa encryption failed");
    }

    return encrypted_message;
}


Bytes RsaPublicKey::encryptWithAes(const Bytes& message) const
{
    base::AesKey symmetric_key(base::AesKey::KeyType::K256BIT);
    if(maxEncryptSize() < symmetric_key.size()) {
        symmetric_key = base::AesKey(base::AesKey::KeyType::K128BIT);
        if(maxEncryptSize() < symmetric_key.size()) {
            RAISE_ERROR(CryptoError, "small Rsa key size for RsaAes encryption");
        }
    }

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
    if(encrypted_message.size() != _encrypted_message_size) {
        RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
    }

    Bytes decrypted_message(_encrypted_message_size);
    auto message_size = RSA_public_decrypt(encrypted_message.size(), encrypted_message.toArray(),
        decrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_PADDING);
    if(message_size == -1) {
        RAISE_ERROR(CryptoError, "rsa decryption failed");
    }

    return decrypted_message.takePart(0, message_size);
}


std::size_t RsaPublicKey::maxEncryptSize() const noexcept
{
    return _encrypted_message_size - ASYMMETRIC_DIFFERENCE;
}


void RsaPublicKey::save(const std::filesystem::path& path) const
{
    writeFile(path, toBytes());
}


RsaPublicKey RsaPublicKey::read(const std::filesystem::path& path)
{
    return RsaPublicKey(readAllFile(path));
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


std::unique_ptr<RSA, decltype(&::RSA_free)> RsaPublicKey::loadKey(const Bytes& key_word)
{
    std::unique_ptr<BIO, decltype(&::BIO_free)> bio(BIO_new_mem_buf(key_word.toArray(), key_word.size()), ::BIO_free);
    RSA* rsa_key = nullptr;
    if(!PEM_read_bio_RSAPublicKey(bio.get(), &rsa_key, nullptr, nullptr)) {
        RAISE_ERROR(CryptoError, "Fail to read public RSA key");
    }
    return std::unique_ptr<RSA, decltype(&::RSA_free)>(rsa_key, ::RSA_free);
}


RsaPublicKey RsaPublicKey::deserialize(base::SerializationIArchive& ia)
{
    base::Bytes bytes;
    ia >> bytes;
    return {bytes};
}


void RsaPublicKey::serialize(base::SerializationOArchive& oa)
{
    oa << toBytes();
}


RsaPrivateKey::RsaPrivateKey(const base::Bytes& key_word)
    : _rsa_key(loadKey(key_word)), _encrypted_message_size(RSA_size(_rsa_key.get()))
{}


Bytes RsaPrivateKey::encrypt(const Bytes& message) const
{
    if(message.size() > maxEncryptSize()) {
        RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
    }

    Bytes encrypted_message(_encrypted_message_size);
    if(!RSA_private_encrypt(
           message.size(), message.toArray(), encrypted_message.toArray(), _rsa_key.get(), RSA_PKCS1_PADDING)) {
        RAISE_ERROR(CryptoError, "rsa encryption failed");
    }

    return encrypted_message;
}


Bytes RsaPrivateKey::decrypt(const Bytes& encrypted_message) const
{
    if(encrypted_message.size() != _encrypted_message_size) {
        RAISE_ERROR(InvalidArgument, "large message size for RSA encryption");
    }

    Bytes decrypt_message(_encrypted_message_size);
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
    return _encrypted_message_size - ASYMMETRIC_DIFFERENCE;
}


void RsaPrivateKey::save(const std::filesystem::path& path) const
{
    writeFile(path, toBytes());
}

RsaPrivateKey RsaPrivateKey::read(const std::filesystem::path& path)
{
    return RsaPrivateKey(readAllFile(path));
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
    static constexpr std::size_t minimal_secure_key_size = 1024;
    if(keys_size < minimal_secure_key_size) {
        LOG_WARNING << "using less than " << minimal_secure_key_size << " bits for key generation is not secure";
    }
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


AesKey::AesKey(const Bytes& bytes)
{
    // key may be 2 * size iv
    static constexpr std::size_t _aes_256_size = static_cast<std::size_t>(base::AesKey::KeyType::K256BIT) / 2 * 3;
    static constexpr std::size_t _aes_128_size = static_cast<std::size_t>(base::AesKey::KeyType::K128BIT) / 2 * 3;

    switch(bytes.size()) {
        case _aes_256_size:
            _type = base::AesKey::KeyType::K256BIT;
            break;
        case _aes_128_size:
            _type = base::AesKey::KeyType::K128BIT;
            break;
        default:
            RAISE_ERROR(InvalidArgument, "bytes_key are not valid. They must be obtained by Key::toBytes");
    }
    auto last_bit_number = static_cast<std::size_t>(_type);
    _key = bytes.takePart(0, last_bit_number);
    _iv = bytes.takePart(last_bit_number, bytes.size());
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


std::size_t AesKey::size() const
{
    switch(_type) {
        case KeyType::K256BIT:
        case KeyType::K128BIT:
            return static_cast<std::size_t>(_type);
        default:
            RAISE_ERROR(CryptoError, "Unexpected key type");
    }
}


void AesKey::save(const std::filesystem::path& path)
{
    writeFile(path, toBytes());
}


AesKey AesKey::read(const std::filesystem::path& path)
{
    return AesKey(readAllFile(path));
}


Bytes AesKey::generateKey(KeyType type)
{
    switch(type) {
        case KeyType::K256BIT:
        case KeyType::K128BIT:
            return generate_bytes(static_cast<std::size_t>(type));
        default:
            RAISE_ERROR(CryptoError, "Unexpected key type");
    }
}


Bytes AesKey::generateIv(KeyType type)
{
    switch(type) {
        case KeyType::K256BIT:
        case KeyType::K128BIT:
            return generate_bytes(static_cast<std::size_t>(type) / 2);
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


base::Bytes base64Encode(const base::Bytes& bytes)
{
    if(bytes.size() == 0) {
        return base::Bytes();
    }

    BIO* bio_temp = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BUF_MEM* bufferPtr = nullptr;
    
    std::unique_ptr<BIO, decltype(&::BIO_free_all)> bio(BIO_push(b64, bio_temp), ::BIO_free_all);
    BIO_set_flags(bio.get(), BIO_FLAGS_BASE64_NO_NL);
    if(BIO_write(bio.get(), bytes.toArray(), static_cast<int>(bytes.size())) < 1) {
        RAISE_ERROR(CryptoError, "Base64 encode write error");
    }
    if(BIO_flush(bio.get()) < 1) {
        RAISE_ERROR(CryptoError, "Base64 encode flush error");
    }
    if(BIO_get_mem_ptr(bio.get(), &bufferPtr) < 1) {
        if(bufferPtr) {
            BUF_MEM_free(bufferPtr);
        }
        RAISE_ERROR(CryptoError, "Get pointer to memory from base64 error");
    }

    base::Bytes base64_bytes(std::string(bufferPtr->data, bufferPtr->length));

    BUF_MEM_free(bufferPtr);
    BIO_set_close(bio.get(), BIO_NOCLOSE);
    return base64_bytes;
}


base::Bytes base64Decode(const base::Bytes& base64_bytes)
{
    if(base64_bytes.size() == 0) {
        return base::Bytes();
    }

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new_mem_buf(base64_bytes.toArray(), base64_bytes.size());
    base::Bytes ret(base64_bytes.size());

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    auto new_length = BIO_read(bio, ret.toArray(), base64_bytes.size());
    if(new_length < 1) {
        BIO_free_all(bio);
        RAISE_ERROR(CryptoError, "Base64 decode read error");
    }

    BIO_free_all(bio);
    return ret.takePart(0, new_length);
}


KeyVault::KeyVault(const base::PropertyTree& config)
{
    auto public_key_path = config.get<std::string>("keys.public_path");
    auto private_key_path = config.get<std::string>("keys.private_path");

    if(std::filesystem::exists(public_key_path) && std::filesystem::exists(private_key_path)) {
        _public_key = std::make_unique<base::RsaPublicKey>(base::RsaPublicKey::read(public_key_path));
        _private_key = std::make_unique<base::RsaPrivateKey>(base::RsaPrivateKey::read(private_key_path));
    }
    else {
        LOG_WARNING << "Key files was not found: public[" << public_key_path << "], private[" << private_key_path
                    << "].";
        static constexpr std::size_t rsa_keys_length = 1000;
        auto keys = base::generateKeys(rsa_keys_length);
        _public_key = std::make_unique<base::RsaPublicKey>(std::move(keys.first));
        _private_key = std::make_unique<base::RsaPrivateKey>(std::move(keys.second));
        _public_key->save(public_key_path);
        _private_key->save(private_key_path);
        LOG_WARNING << "Generated new key pair and saved by config paths.";
    }
    LOG_INFO << "Public key hash: " << base::Sha256::compute(_public_key->toBytes()).toHex();
    // TODO: maybe implement unload to disk mechanic for private key.
}


const base::RsaPublicKey& KeyVault::getPublicKey() const noexcept
{
    return *_public_key;
}


} // namespace base
