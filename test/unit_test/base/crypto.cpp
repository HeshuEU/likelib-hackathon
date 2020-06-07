#include <boost/test/unit_test.hpp>

#include <openssl/evp.h>

#include <base/crypto.hpp>

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_256bit)
{
    base::Bytes msg("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey key;
    auto encrypted_data = key.encrypt(msg);
    auto decrypt_target = key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(msg.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_256bit_by_serialized_key)
{
    base::Bytes msg("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey key;
    auto encrypted_data = key.encrypt(msg);
    auto serialised_key = key.toBytes();
    base::AesKey deserialized_key(serialised_key);
    auto decrypt_target = deserialized_key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(msg.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_128bit)
{
    base::Bytes msg("dfjbvalgecnhq=ygrbnf5xgvidytnwucgfim2y139sv7yx");
    base::AesKey key(base::AesKey::KeyType::K128BIT);
    auto encrypted_data = key.encrypt(msg);
    auto decrypt_target = key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(msg.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_128bit_by_serialized_key)
{
    base::Bytes msg("djbvalcnhq=ygrn3f5xvidytwugfim2yx19vyx");
    base::AesKey key(base::AesKey::KeyType::K128BIT);
    auto encrypted_data = key.encrypt(msg);
    auto serialised_key = key.toBytes();
    base::AesKey deserialized_key(serialised_key);
    auto decrypt_target = deserialized_key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(msg.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_double_encrypt_128bit)
{
    base::Bytes msg("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey key(base::AesKey::KeyType ::K128BIT);
    auto encrypted_data_1 = key.encrypt(msg);
    auto encrypted_data_2 = key.encrypt(msg);
    BOOST_CHECK_EQUAL(encrypted_data_1.toString(), encrypted_data_2.toString());
}


BOOST_AUTO_TEST_CASE(aes_serialization_256bit)
{
    base::Bytes target_msg("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey target_key(base::AesKey::KeyType ::K128BIT);
    auto encrypted_data = target_key.encrypt(target_msg);

    std::filesystem::path key_path{ "test.aes" };
    target_key.save(key_path);

    auto deserialized_key = base::AesKey::read(key_path);
    auto decrypt_message = deserialized_key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_msg, decrypt_message);

    std::filesystem::remove(key_path);
}


BOOST_AUTO_TEST_CASE(secp256_sign_verify)
{
    auto private_key = base::Secp256PrivateKey{};
    auto hash = base::Sha256::compute(base::Bytes("111"));
    auto signature = private_key.sign(hash.getBytes().toBytes());
    auto public_key = private_key.toPublicKey();
    BOOST_CHECK_EQUAL(private_key.decodeSignatureToPublicKey(signature, hash.getBytes().toBytes()).toBytes(),
                      public_key.toBytes());
}


BOOST_AUTO_TEST_CASE(secp256_save_load)
{
    auto private_key1 = base::Secp256PrivateKey{};
    std::filesystem::path private_key_path{ "ssh/secp" };

    private_key1.save(private_key_path);

    auto private_key2 = base::Secp256PrivateKey::load(private_key_path);
    auto hash = base::Sha256::compute(base::Bytes("555"));

    auto signature1 = private_key1.sign(hash.getBytes().toBytes());
    auto signature2 = private_key2.sign(hash.getBytes().toBytes());

    auto public_key1 = private_key1.toPublicKey();
    auto public_key2 = private_key2.toPublicKey();

    BOOST_CHECK_EQUAL(private_key1.decodeSignatureToPublicKey(signature1, hash.getBytes().toBytes()).toBytes(),
                      public_key1.toBytes());
    BOOST_CHECK_EQUAL(private_key2.decodeSignatureToPublicKey(signature2, hash.getBytes().toBytes()).toBytes(),
                      public_key2.toBytes());

    BOOST_CHECK_EQUAL(private_key1.decodeSignatureToPublicKey(signature2, hash.getBytes().toBytes()).toBytes(),
                      public_key2.toBytes());
    BOOST_CHECK_EQUAL(private_key2.decodeSignatureToPublicKey(signature1, hash.getBytes().toBytes()).toBytes(),
                      public_key1.toBytes());

    std::filesystem::remove(private_key_path);
}

BOOST_AUTO_TEST_CASE(secp256_serialization)
{
    auto private_key1 = base::Secp256PrivateKey{};
    std::filesystem::path private_key_path{ "ssh/secp" };

    base::SerializationOArchive oa;
    oa.serialize(private_key1);
    base::SerializationIArchive ia(oa.getBytes());
    auto private_key2 = ia.deserialize<base::Secp256PrivateKey>();

    auto hash = base::Sha256::compute(base::Bytes("555"));

    auto signature1 = private_key1.sign(hash.getBytes().toBytes());
    auto signature2 = private_key2.sign(hash.getBytes().toBytes());

    auto public_key1 = private_key1.toPublicKey();
    auto public_key2 = private_key2.toPublicKey();

    BOOST_CHECK_EQUAL(private_key1.decodeSignatureToPublicKey(signature1, hash.getBytes().toBytes()).toBytes(),
                      public_key1.toBytes());
    BOOST_CHECK_EQUAL(private_key2.decodeSignatureToPublicKey(signature2, hash.getBytes().toBytes()).toBytes(),
                      public_key2.toBytes());

    BOOST_CHECK_EQUAL(private_key1.decodeSignatureToPublicKey(signature2, hash.getBytes().toBytes()).toBytes(),
                      public_key2.toBytes());
    BOOST_CHECK_EQUAL(private_key2.decodeSignatureToPublicKey(signature1, hash.getBytes().toBytes()).toBytes(),
                      public_key1.toBytes());

    std::filesystem::remove(private_key_path);
}