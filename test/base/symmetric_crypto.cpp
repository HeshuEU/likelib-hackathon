#include <boost/test/unit_test.hpp>
#include <base/error.hpp>

#include "base/symmetric_crypto.hpp"


/*BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_256bit)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::aes::AesKey key;
    auto encrypted_data = key.encrypt(target_bytes);
    auto decrypt_target = key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_bytes.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_256bit_by_serialized_key)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::aes::AesKey key;
    auto encrypted_data = key.encrypt(target_bytes);
    auto serialised_key = key.toBytes();
    base::aes::AesKey deserialized_key(serialised_key);
    auto decrypt_target = deserialized_key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_bytes.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_128bit)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::aes::AesKey key(base::aes::KeyType::Aes128BitKey);
    auto encrypted_data = key.encrypt(target_bytes);
    auto decrypt_target = key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_bytes.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_128bit_by_serialized_key)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::aes::AesKey key(base::aes::KeyType::Aes128BitKey);
    auto encrypted_data = key.encrypt(target_bytes);
    auto serialised_key = key.toBytes();
    base::aes::AesKey deserialized_key(serialised_key);
    auto decrypt_target = deserialized_key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_bytes.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_double_encrypt_128bit)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::aes::AesKey key(base::aes::KeyType::Aes128BitKey);
    auto encrypted_data_1 = key.encrypt(target_bytes);
    auto encrypted_data_2 = key.encrypt(target_bytes);
    BOOST_CHECK_EQUAL(encrypted_data_1.toString(), encrypted_data_2.toString());
}

BOOST_AUTO_TEST_CASE(aes_deserialization_failed)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidvsdvytnwucgfim2yx139sv7yx");
    BOOST_CHECK_THROW(base::aes::AesKey key(target_bytes), base::InvalidArgument);
}
*/