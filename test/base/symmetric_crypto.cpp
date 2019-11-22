#include <boost/test/unit_test.hpp>

#include "base/symmetric_crypto.hpp"


BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_default)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey key;
    auto encrypted_data = key.encrypt(target_bytes);
    auto decrypt_target = key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_bytes.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_default_by_serialized_key)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey key;
    auto encrypted_data = key.encrypt(target_bytes);
    auto serialised_key = key.toBytes();
    base::AesKey deserialized_key(serialised_key);
    auto decrypt_target = deserialized_key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_bytes.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_128bit)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey key(base::KeyType::Aes128BitKey);
    auto encrypted_data = key.encrypt(target_bytes);
    auto decrypt_target = key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_bytes.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_128bit_by_serialized_key)
{
    base::Bytes target_bytes("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey key(base::KeyType::Aes128BitKey);
    auto encrypted_data = key.encrypt(target_bytes);
    auto serialised_key = key.toBytes();
    base::AesKey deserialized_key(serialised_key);
    auto decrypt_target = deserialized_key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_bytes.toString(), decrypt_target.toString());
}