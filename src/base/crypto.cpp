#include "crypto.hpp"

#include "base/assert.hpp"
#include "base/directory.hpp"
#include "base/error.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"
#include "base/property_tree.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <include/secp256k1.h>
#include <include/secp256k1_recovery.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>

namespace
{

std::string readAllFile(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        RAISE_ERROR(base::InvalidArgument, "the file with this path does not exist");
    }
    std::ifstream file(path, std::ifstream::binary);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
    return buffer;
}


void writeFile(const std::filesystem::path& path, const std::string& data)
{
    if (std::filesystem::exists(path)) {
        LOG_WARNING << "file[" << path << "] already exists";
    }
    else {
        auto dir = path.parent_path();
        if (!dir.empty()) {
            base::createIfNotExists(dir);
        }
    }

    std::ofstream file(path, std::ofstream::binary);
    if (!file.is_open()) {
        RAISE_ERROR(base::InvalidArgument, "Failed to create file.");
    }
    file.write(data.c_str(), data.size());
}


// generates a cryptographically safe byte sequence, see issue 93
base::Bytes generate_bytes(std::size_t size)
{
    base::Bytes data(size);
    RAND_bytes(data.getData(), static_cast<int>(size));
    return data;
}


base::Secp256PrivateKey loadOrGenerateKey(const std::string_view& keys_dir_path)
{
    auto private_key_path = base::config::makePrivateKeyPath(keys_dir_path);

    if (std::filesystem::exists(private_key_path)) {
        return base::Secp256PrivateKey::load(private_key_path);
    }
    LOG_INFO << "Key files were not found by path " << keys_dir_path << ". Generating new keypair";
    auto key = base::Secp256PrivateKey();
    key.save(private_key_path);
    return key;
}

} // namespace


namespace base
{

AesKey::AesKey()
  : _type(KeyType::K256BIT)
  , _key(generateKey(KeyType::K256BIT))
  , _iv(generateIv())
{}


AesKey::AesKey(KeyType type)
  : _type(type)
  , _key(generateKey(type))
  , _iv(generateIv())
{}


AesKey::AesKey(const Bytes& bytes)
{
    static constexpr std::size_t _aes_256_size = 48;
    static constexpr std::size_t _aes_128_size = 32;

    switch (bytes.size()) {
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
    _iv = FixedBytes<16>(bytes.takePart(last_bit_number, bytes.size()));
}


Bytes AesKey::toBytes() const
{
    return Bytes(_key.toString() + _iv.toString());
}


Bytes AesKey::encrypt(const Bytes& data) const
{
    switch (_type) {
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
    switch (_type) {
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
    switch (_type) {
        case KeyType::K256BIT:
        case KeyType::K128BIT:
            return static_cast<std::size_t>(_type);
        default:
            RAISE_ERROR(CryptoError, "Unexpected key type");
    }
}


void AesKey::save(const std::filesystem::path& path)
{
    writeFile(path, toBytes().toString());
}


AesKey AesKey::read(const std::filesystem::path& path)
{
    return AesKey(base::Bytes(readAllFile(path)));
}


Bytes AesKey::generateKey(KeyType type)
{
    switch (type) {
        case KeyType::K256BIT:
        case KeyType::K128BIT:
            return generate_bytes(static_cast<std::size_t>(type));
        default:
            RAISE_ERROR(CryptoError, "Unexpected key type");
    }
}


Bytes AesKey::generateIv()
{
    return generate_bytes(iv_cbc_size);
}


Bytes AesKey::encrypt256Aes(const Bytes& data) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if (1 != EVP_EncryptInit_ex(context.get(), EVP_aes_256_cbc(), nullptr, _key.getData(), _iv.getData())) {
        RAISE_ERROR(CryptoError, "failed to initialize context");
    }

    Bytes output_data(data.size() * 2);

    int current_data_len = 0;
    if (1 != EVP_EncryptUpdate(context.get(), output_data.getData(), &current_data_len, data.getData(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to encrypt message");
    }
    int encrypted_message_len_in_buffer = current_data_len;

    if (1 != EVP_EncryptFinal_ex(context.get(), output_data.getData() + current_data_len, &current_data_len)) {
        RAISE_ERROR(CryptoError, "unable to finalize encrypt");
    }
    encrypted_message_len_in_buffer += current_data_len;

    return output_data.takePart(0, encrypted_message_len_in_buffer);
}


base::Bytes AesKey::decrypt256Aes(const base::Bytes& data) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if (1 != EVP_DecryptInit_ex(context.get(), EVP_aes_256_cbc(), nullptr, _key.getData(), _iv.getData())) {
        RAISE_ERROR(CryptoError, "failed to initialize context");
    }

    Bytes output_data(data.size() * 2);

    int current_data_len = 0;
    if (1 != EVP_DecryptUpdate(context.get(), output_data.getData(), &current_data_len, data.getData(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to decrypt message");
    }
    int decrypted_message_len_in_buffer = current_data_len;

    if (1 != EVP_DecryptFinal_ex(context.get(), output_data.getData() + current_data_len, &current_data_len)) {
        RAISE_ERROR(CryptoError, "unable to finalize decrypt");
    }
    decrypted_message_len_in_buffer += current_data_len;

    return output_data.takePart(0, decrypted_message_len_in_buffer);
}


base::Bytes AesKey::encrypt128Aes(const base::Bytes& data) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if (1 != EVP_EncryptInit_ex(context.get(), EVP_aes_128_cbc(), nullptr, _key.getData(), _iv.getData())) {
        RAISE_ERROR(CryptoError, "failed to initialize context");
    }

    Bytes output_data(data.size() * 2);

    int current_data_len = 0;
    if (1 != EVP_EncryptUpdate(context.get(), output_data.getData(), &current_data_len, data.getData(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to encrypt message");
    }
    int encrypted_message_len_in_buffer = current_data_len;

    if (1 != EVP_EncryptFinal_ex(context.get(), output_data.getData() + current_data_len, &current_data_len)) {
        RAISE_ERROR(CryptoError, "unable to finalize encrypt");
    }
    encrypted_message_len_in_buffer += current_data_len;

    return output_data.takePart(0, encrypted_message_len_in_buffer);
}


base::Bytes AesKey::decrypt128Aes(const base::Bytes& data) const
{
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    if (1 != EVP_DecryptInit_ex(context.get(), EVP_aes_128_cbc(), nullptr, _key.getData(), _iv.getData())) {
        RAISE_ERROR(CryptoError, "failed to initialize context");
    }

    Bytes output_data(data.size() * 2);

    int current_data_len = 0;
    if (1 != EVP_DecryptUpdate(context.get(), output_data.getData(), &current_data_len, data.getData(), data.size()))
        RAISE_ERROR(CryptoError, "failed to decrypt message");
    int decrypted_message_len_in_buffer = current_data_len;

    if (1 != EVP_DecryptFinal_ex(context.get(), output_data.getData() + current_data_len, &current_data_len))
        RAISE_ERROR(CryptoError, "unable to finalize decrypt");
    decrypted_message_len_in_buffer += current_data_len;

    return output_data.takePart(0, decrypted_message_len_in_buffer);
}


Secp256PrivateKey::Secp256PrivateKey()
  : _secp_key(generate_bytes(SECP256_PRIVATE_KEY_SIZE))
{
    if (!is_valid()) {
        RAISE_ERROR(base::CryptoError, "error at creation secp256k1 key");
    }
}


Secp256PrivateKey::Secp256PrivateKey(const base::FixedBytes<SECP256_PRIVATE_KEY_SIZE>& private_key_bytes)
  : _secp_key(private_key_bytes)
{
    if (!is_valid()) {
        RAISE_ERROR(base::CryptoError, "error at creation secp256k1 key");
    }
}


bool Secp256PrivateKey::is_valid() const
{
    std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> context(
      secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY), secp256k1_context_destroy);
    return secp256k1_ec_seckey_verify(context.get(), _secp_key.getData()) == 1;
}


base::FixedBytes<Secp256PrivateKey::SECP256_PUBLIC_KEY_SIZE> Secp256PrivateKey::toPublicKey() const
{
    std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> context(
      secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY), secp256k1_context_destroy);
    secp256k1_pubkey pubkey;
    if (secp256k1_ec_pubkey_create(context.get(), &pubkey, _secp_key.toBytes().getData()) == 0) {
        RAISE_ERROR(base::CryptoError, "secret key for create public key is invalid");
    }

    base::FixedBytes<SECP256_PUBLIC_KEY_SIZE> output;
    std::size_t output_size = output.size();

    secp256k1_ec_pubkey_serialize(context.get(), output.getData(), &output_size, &pubkey, SECP256K1_EC_UNCOMPRESSED);
    if (output_size == 0) {
        RAISE_ERROR(base::CryptoError, "secret key for create public key is invalid");
    }

    return output;
}


Secp256PrivateKey::Signature Secp256PrivateKey::sign(const base::Bytes& bytes_to_sign) const
{
    auto hash = base::Sha256::compute(bytes_to_sign);
    std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> context(
      secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY), secp256k1_context_destroy);
    secp256k1_ecdsa_recoverable_signature recoverable_signature;
    if (secp256k1_ecdsa_sign_recoverable(
          context.get(), &recoverable_signature, hash.getBytes().getData(), _secp_key.getData(), nullptr, nullptr) ==
        0) {
        RAISE_ERROR(base::CryptoError, "error signing bytes");
    }
    int rec_id = -1;
    base::Bytes serialized_recoverable_signature(64);
    secp256k1_ecdsa_recoverable_signature_serialize_compact(
      context.get(), serialized_recoverable_signature.getData(), &rec_id, &recoverable_signature);

    if (rec_id == -1) {
        RAISE_ERROR(base::CryptoError, "signature serialization failed");
    }
    serialized_recoverable_signature.append(static_cast<base::Byte>(rec_id));
    return Signature{ serialized_recoverable_signature };
}


base::FixedBytes<Secp256PrivateKey::SECP256_PUBLIC_KEY_SIZE> Secp256PrivateKey::decodeSignatureToPublicKey(
  const Signature& signature,
  const base::Bytes& bytes_to_check)
{
    auto hash = base::Sha256::compute(bytes_to_check);
    std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)> context(
      secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY), secp256k1_context_destroy);

    auto sig_data = signature.toBytes();
    secp256k1_ecdsa_recoverable_signature recoverable_signature;
    if (secp256k1_ecdsa_recoverable_signature_parse_compact(
          context.get(), &recoverable_signature, sig_data.getData(), static_cast<int>(sig_data[sig_data.size() - 1])) ==
        0) {
        RAISE_ERROR(base::CryptoError, "could not parsed signature");
    }

    secp256k1_pubkey pubkey;
    if (secp256k1_ecdsa_recover(context.get(), &pubkey, &recoverable_signature, hash.getBytes().getData()) == 0) {
        RAISE_ERROR(base::CryptoError, "recover public key is invalid");
    }

    base::FixedBytes<SECP256_PUBLIC_KEY_SIZE> output;
    std::size_t output_size = output.size();

    secp256k1_ec_pubkey_serialize(context.get(), output.getData(), &output_size, &pubkey, SECP256K1_EC_UNCOMPRESSED);
    if (output_size == 0) {
        RAISE_ERROR(base::CryptoError, "secret key for create public key is invalid");
    }

    return output;
}


void Secp256PrivateKey::save(const std::filesystem::path& path) const
{
    writeFile(path, base::toHex(_secp_key));
}


Secp256PrivateKey Secp256PrivateKey::load(const std::filesystem::path& path)
{
    return Secp256PrivateKey(base::fromHex<base::FixedBytes<SECP256_PRIVATE_KEY_SIZE>>(readAllFile(path)));
}


Secp256PrivateKey Secp256PrivateKey::deserialize(base::SerializationIArchive& ia)
{
    auto bytes = ia.deserialize<FixedBytes<SECP256_PRIVATE_KEY_SIZE>>();
    return { bytes };
}


void Secp256PrivateKey::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_secp_key);
}


const base::FixedBytes<Secp256PrivateKey::SECP256_PRIVATE_KEY_SIZE>& Secp256PrivateKey::getBytes() const
{
    return _secp_key;
}


KeyVault::KeyVault(const base::PropertyTree& config)
{
    auto keys_folder = config.get<std::string>("keys_dir");
    _key = loadOrGenerateKey(keys_folder);
}


KeyVault::KeyVault(const std::string_view& keys_folder)
{
    _key = loadOrGenerateKey(keys_folder);
}


const base::Secp256PrivateKey& KeyVault::getKey() const noexcept
{
    return _key;
}

} // namespace base
