#include "symmetric_crypto.hpp"

#include "base/error.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <vector>
#include <cstring>
#include <memory>


namespace
{

base::Bytes generate_bytes(std::size_t size)
{
    std::vector<base::Byte> data(size);
    RAND_bytes(data.data(), static_cast<int>(size));
    return base::Bytes(data);
}

base::Bytes encrypt256Aes(const base::Bytes& data, const base::Bytes& iv, const base::Bytes& key)
{
    if(key.size() != 256 / 8 || iv.size() != 128 / 8) {
        RAISE_ERROR(base::InvalidArgument, "key or iv is not a valid size");
    }

    // prepare buffer
    std::size_t encrypted_buffer_full_len = data.size() * 2;
    unsigned char encrypted_buffer[encrypted_buffer_full_len]; // encrypted data buffer
    std::memset(encrypted_buffer, 0, encrypted_buffer_full_len);

    // Create and initialise the context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> encrypt_context(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    // Initialise the encryption operation. IMPORTANT - ensure you use a key
    // and IV size appropriate for your cipher
    // In this example we are using 256 bit AES (i.e. a 256 bit key). The
    // IV size for *most* modes is the same as the block size. For AES this
    // is 128 bits
    if(1 != EVP_EncryptInit_ex(encrypt_context.get(), EVP_aes_256_cbc(), NULL, key.toArray(), iv.toArray())) {
        RAISE_ERROR(base::InvalidArgument, "Error in context initialization");
    }

    // Provide the message to be encrypted, and obtain the encrypted output.
    // EVP_EncryptUpdate can be called multiple times if necessary
    int current_data_len = 0;
    auto encrypt_exit_code =
        EVP_EncryptUpdate(encrypt_context.get(), encrypted_buffer, &current_data_len, data.toArray(), data.size());
    if(1 != encrypt_exit_code) {
        RAISE_ERROR(base::InvalidArgument, "Error in encrypt");
    }
    int encrypted_message_len_in_buffer = current_data_len;


    // Finalise the encryption. Further encrypted_buffer bytes may be written at
    // this stage.
    auto encrypt_final_exit_code =
        EVP_EncryptFinal_ex(encrypt_context.get(), encrypted_buffer + current_data_len, &current_data_len);
    if(1 != encrypt_final_exit_code) {
        RAISE_ERROR(base::InvalidArgument, "Error in finalization");
    }
    encrypted_message_len_in_buffer += current_data_len;

    // copy form buffer
    std::vector<base::Byte> output_data(encrypted_message_len_in_buffer);
    std::memcpy(output_data.data(), encrypted_buffer, output_data.size());
    return base::Bytes(output_data);
}

base::Bytes decrypt256Aes(const base::Bytes& data, const base::Bytes& iv, const base::Bytes& key)
{
    if(key.size() != 256 / 8 || iv.size() != 128 / 8) {
        RAISE_ERROR(base::InvalidArgument, "key or iv is not a valid size");
    }

    std::size_t decrypted_buffer_full_len = data.size() * 2;
    unsigned char decrypted_buffer[decrypted_buffer_full_len];
    std::memset(decrypted_buffer, 0, decrypted_buffer_full_len);

    // Create and initialise the context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> encrypt_context(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    // Initialise the decryption operation. IMPORTANT - ensure you use a key
    // and IV size appropriate for your cipher
    // In this example we are using 256 bit AES (i.e. a 256 bit key). The
    // IV size for *most* modes is the same as the block size. For AES this
    // is 128 bits
    if(1 != EVP_DecryptInit_ex(encrypt_context.get(), EVP_aes_256_cbc(), NULL, key.toArray(), iv.toArray())) {
        RAISE_ERROR(base::InvalidArgument, "Error in context initialization");
    }

    // Provide the message to be decrypted, and obtain the decrypted_buffer output.
    // EVP_DecryptUpdate can be called multiple times if necessary.
    int current_data_len = 0;
    if(1 != EVP_DecryptUpdate(encrypt_context.get(), decrypted_buffer, &current_data_len, data.toArray(), data.size()))
        RAISE_ERROR(base::InvalidArgument, "Error in decrypt");
    int decrypted_message_len_in_buffer = current_data_len;

    // Finalise the decryption. Further decrypted_buffer bytes may be written at
    // this stage.
    if(1 != EVP_DecryptFinal_ex(encrypt_context.get(), decrypted_buffer + current_data_len, &current_data_len))
        RAISE_ERROR(base::InvalidArgument, "Error in finalization");
    decrypted_message_len_in_buffer += current_data_len;

    std::vector<base::Byte> output_data(decrypted_message_len_in_buffer);
    std::memcpy(output_data.data(), decrypted_buffer, output_data.size());
    return base::Bytes(output_data);
}

base::Bytes encrypt128Aes(const base::Bytes& data, const base::Bytes& iv, const base::Bytes& key)
{
    if(key.size() != 128 / 8 || iv.size() != 64 / 8) {
        RAISE_ERROR(base::InvalidArgument, "key or iv is not a valid size");
    }

    // prepare buffer
    std::size_t encrypted_buffer_full_len = data.size() * 2;
    unsigned char encrypted_buffer[encrypted_buffer_full_len]; // encrypted data buffer
    std::memset(encrypted_buffer, 0, encrypted_buffer_full_len);

    // Create and initialise the context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> encrypt_context(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    // Initialise the encryption operation. IMPORTANT - ensure you use a key
    // and IV size appropriate for your cipher
    // In this example we are using 256 bit AES (i.e. a 256 bit key). The
    // IV size for *most* modes is the same as the block size. For AES this
    // is 128 bits
    if(1 != EVP_EncryptInit_ex(encrypt_context.get(), EVP_aes_128_cbc(), NULL, key.toArray(), iv.toArray())) {
        RAISE_ERROR(base::InvalidArgument, "Error in context initialization");
    }

    // Provide the message to be encrypted, and obtain the encrypted output.
    // EVP_EncryptUpdate can be called multiple times if necessary
    int current_data_len = 0;
    auto encrypt_exit_code =
        EVP_EncryptUpdate(encrypt_context.get(), encrypted_buffer, &current_data_len, data.toArray(), data.size());
    if(1 != encrypt_exit_code) {
        RAISE_ERROR(base::InvalidArgument, "Error in encrypt");
    }
    int encrypted_message_len_in_buffer = current_data_len;


    // Finalise the encryption. Further encrypted_buffer bytes may be written at
    // this stage.
    auto encrypt_final_exit_code =
        EVP_EncryptFinal_ex(encrypt_context.get(), encrypted_buffer + current_data_len, &current_data_len);
    if(1 != encrypt_final_exit_code) {
        RAISE_ERROR(base::InvalidArgument, "Error in finalization");
    }
    encrypted_message_len_in_buffer += current_data_len;

    // copy form buffer
    std::vector<base::Byte> output_data(encrypted_message_len_in_buffer);
    std::memcpy(output_data.data(), encrypted_buffer, output_data.size());
    return base::Bytes(output_data);
}

base::Bytes decrypt128Aes(const base::Bytes& data, const base::Bytes& iv, const base::Bytes& key)
{
    if(key.size() != 128 / 8 || iv.size() != 64 / 8) {
        RAISE_ERROR(base::InvalidArgument, "key or iv is not a valid size");
    }

    std::size_t decrypted_buffer_full_len = data.size() * 2;
    unsigned char decrypted_buffer[decrypted_buffer_full_len];
    std::memset(decrypted_buffer, 0, decrypted_buffer_full_len);

    // Create and initialise the context
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> encrypt_context(
        EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

    // Initialise the decryption operation. IMPORTANT - ensure you use a key
    // and IV size appropriate for your cipher
    // In this example we are using 256 bit AES (i.e. a 256 bit key). The
    // IV size for *most* modes is the same as the block size. For AES this
    // is 128 bits
    if(1 != EVP_DecryptInit_ex(encrypt_context.get(), EVP_aes_128_cbc(), NULL, key.toArray(), iv.toArray())) {
        RAISE_ERROR(base::InvalidArgument, "Error in context initialization");
    }

    // Provide the message to be decrypted, and obtain the decrypted_buffer output.
    // EVP_DecryptUpdate can be called multiple times if necessary.
    int current_data_len = 0;
    if(1 != EVP_DecryptUpdate(encrypt_context.get(), decrypted_buffer, &current_data_len, data.toArray(), data.size()))
        RAISE_ERROR(base::InvalidArgument, "Error in decrypt");
    int decrypted_message_len_in_buffer = current_data_len;

    // Finalise the decryption. Further decrypted_buffer bytes may be written at
    // this stage.
    if(1 != EVP_DecryptFinal_ex(encrypt_context.get(), decrypted_buffer + current_data_len, &current_data_len))
        RAISE_ERROR(base::InvalidArgument, "Error in finalization");
    decrypted_message_len_in_buffer += current_data_len;

    std::vector<base::Byte> output_data(decrypted_message_len_in_buffer);
    std::memcpy(output_data.data(), decrypted_buffer, output_data.size());
    return base::Bytes(output_data);
}

} // namespace

namespace base
{

namespace aes
{

    Key::Key()
        : _type(KeyType::Aes256BitKey), _key(generateKey(KeyType::Aes256BitKey)), _iv(generateIv(KeyType::Aes256BitKey))
    {}

    Key::Key(KeyType type) : _type(type), _key(generateKey(type)), _iv(generateIv(type))
    {}

    Key::Key(const Bytes& bytes_key)
    {
        if((bytes_key.size() % 8 != 0) ||
            (bytes_key.size() % 3) != 0) { // multiple bit and concatenate size = iv.size() * 3
            RAISE_ERROR(InvalidArgument, "key data are not valid");
        }
        switch(bytes_key.size() / 3) {
            case 16:
                _type = KeyType::Aes256BitKey;
                _key = bytes_key.takePart(0, 16 * 2);
                _iv = bytes_key.takePart(16 * 2, bytes_key.size());
                break;
            case 8:
                _type = KeyType::Aes128BitKey;
                _key = bytes_key.takePart(0, 8 * 2);
                _iv = bytes_key.takePart(8 * 2, bytes_key.size());
                break;
            default:
                RAISE_ERROR(InvalidArgument, "key data are not valid");
        }
    }

    Bytes Key::toBytes() const
    {
        return Bytes(_key.toString() + _iv.toString()); // concatenate size = iv.size() * 3
    }

    Bytes Key::encrypt(const Bytes& data) const
    {
        switch(_type) {
            case KeyType::Aes256BitKey:
                return encrypt256Aes(data, _iv, _key);
            case KeyType::Aes128BitKey:
                return encrypt128Aes(data, _iv, _key);
            default:
                RAISE_ERROR(Error, "Unexpected key type");
        }
    }

    Bytes Key::decrypt(const Bytes& data) const
    {
        switch(_type) {
            case KeyType::Aes256BitKey:
                return decrypt256Aes(data, _iv, _key);
            case KeyType::Aes128BitKey:
                return decrypt128Aes(data, _iv, _key);
            default:
                RAISE_ERROR(Error, "Unexpected key type");
        }
    }

    Bytes Key::generateKey(KeyType type)
    {
        switch(type) {
            case KeyType::Aes256BitKey:
                return generate_bytes(16 * 2); // 32(bytes) * 8(bit in byte) = 256(bit)
            case KeyType::Aes128BitKey:
                return generate_bytes(8 * 2); // 16(bytes) * 8(bit in byte) = 128(bit)
            default:
                RAISE_ERROR(Error, "Unexpected key type");
        }
    }

    Bytes Key::generateIv(KeyType type)
    {
        switch(type) {
            case KeyType::Aes256BitKey:
                return generate_bytes(16); // 16(bytes) * 8(bit in byte) = 128(bit)
            case KeyType::Aes128BitKey:
                return generate_bytes(8); // 8(bytes) * 8(bit in byte) = 64(bit)
            default:
                RAISE_ERROR(Error, "Unexpected key type");
        }
    }

} // namespace aes

} // namespace base