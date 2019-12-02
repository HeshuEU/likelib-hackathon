#include "symmetric_crypto.hpp"

#include "base/error.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <memory>

namespace
{

base::Bytes generate_bytes(std::size_t size)
{
    std::vector<base::Byte> data(size);
    RAND_bytes(data.data(), static_cast<int>(size));
    return base::Bytes(data);
}

} // namespace

namespace base
{

namespace aes
{

    AesKey::AesKey()
        : _type(KeyType::Aes256BitKey), _key(generateKey(KeyType::Aes256BitKey)), _iv(generateIv(KeyType::Aes256BitKey))
    {}

    AesKey::AesKey(KeyType type) : _type(type), _key(generateKey(type)), _iv(generateIv(type))
    {}

    AesKey::AesKey(const Bytes& bytes_key)
    {
        switch(bytes_key.size()) {
            case _aes_256_size:
                _type = KeyType::Aes256BitKey;
                _key = bytes_key.takePart(0, 16 * 2);
                _iv = bytes_key.takePart(16 * 2, bytes_key.size());
                break;
            case _aes_128_size:
                _type = KeyType::Aes128BitKey;
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
            case KeyType::Aes256BitKey:
                return encrypt256Aes(data);
            case KeyType::Aes128BitKey:
                return encrypt128Aes(data);
            default:
                RAISE_ERROR(CryptoError, "Unexpected key type");
        }
    }

    Bytes AesKey::decrypt(const Bytes& data) const
    {
        switch(_type) {
            case KeyType::Aes256BitKey:
                return decrypt256Aes(data);
            case KeyType::Aes128BitKey:
                return decrypt128Aes(data);
            default:
                RAISE_ERROR(CryptoError, "Unexpected key type");
        }
    }

    Bytes AesKey::generateKey(KeyType type)
    {
        switch(type) {
            case KeyType::Aes256BitKey:
                return generate_bytes(16 * 2); // 32(bytes) * 8(bit in byte) = 256(bit)
            case KeyType::Aes128BitKey:
                return generate_bytes(8 * 2); // 16(bytes) * 8(bit in byte) = 128(bit)
            default:
                RAISE_ERROR(CryptoError, "Unexpected key type");
        }
    }

    Bytes AesKey::generateIv(KeyType type)
    {
        switch(type) {
            case KeyType::Aes256BitKey:
                return generate_bytes(16); // 16(bytes) * 8(bit in byte) = 128(bit)
            case KeyType::Aes128BitKey:
                return generate_bytes(8); // 8(bytes) * 8(bit in byte) = 64(bit)
            default:
                RAISE_ERROR(CryptoError, "Unexpected key type");
        }
    }

    Bytes AesKey::encrypt256Aes(const Bytes& data) const
    {
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(
            EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(1 != EVP_EncryptInit_ex(context.get(), EVP_aes_256_cbc(), NULL, _key.toArray(), _iv.toArray())) {
            RAISE_ERROR(CryptoError, "failed to initialize context");
        }

        Bytes output_data(data.size() * 2);

        int current_data_len = 0;
        if(1 !=
            EVP_EncryptUpdate(context.get(), output_data.toArray(), &current_data_len, data.toArray(), data.size())) {
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
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(
            EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(1 != EVP_DecryptInit_ex(context.get(), EVP_aes_256_cbc(), NULL, _key.toArray(), _iv.toArray())) {
            RAISE_ERROR(CryptoError, "failed to initialize context");
        }

        Bytes output_data(data.size() * 2);

        int current_data_len = 0;
        if(1 !=
            EVP_DecryptUpdate(context.get(), output_data.toArray(), &current_data_len, data.toArray(), data.size())) {
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
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(
            EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(1 != EVP_EncryptInit_ex(context.get(), EVP_aes_128_cbc(), NULL, _key.toArray(), _iv.toArray())) {
            RAISE_ERROR(CryptoError, "failed to initialize context");
        }

        Bytes output_data(data.size() * 2);

        int current_data_len = 0;
        if(1 !=
            EVP_EncryptUpdate(context.get(), output_data.toArray(), &current_data_len, data.toArray(), data.size())) {
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
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(
            EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);

        if(1 != EVP_DecryptInit_ex(context.get(), EVP_aes_128_cbc(), NULL, _key.toArray(), _iv.toArray())) {
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


} // namespace aes

} // namespace base