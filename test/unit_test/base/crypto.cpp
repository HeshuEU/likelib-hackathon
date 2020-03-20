#include <boost/test/unit_test.hpp>

#include <openssl/evp.h>

#include <base/crypto.hpp>

BOOST_AUTO_TEST_CASE(Rsa_pub_encrypt_priv_decrypt_check)
{
    auto rsa = base::generateKeys(3688);

    base::Bytes msg{"RS@_Pub_Priv !EST"};
    auto enc_msg = rsa.first.encrypt(msg);
    auto dec_msg = rsa.second.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(Rsa_priv_encrypt_pub_decrypt_check)
{
    auto rsa = base::generateKeys(2644);

    base::Bytes msg{"RSA_Pr1v Pub_TES!"};
    auto enc_msg = rsa.second.encrypt(msg);
    auto dec_msg = rsa.first.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(Rsa_stress_pub_encrypt_priv_decrypt_check)
{
    auto rsa = base::generateKeys(2048);

    base::Bytes stress_msg(rsa.first.maxEncryptSize());
    auto enc_msg = rsa.first.encrypt(stress_msg);
    auto dec_msg = rsa.second.decrypt(enc_msg);
    BOOST_CHECK(stress_msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(Rsa_stress_priv_encrypt_pub_decrypt_check)
{
    auto rsa = base::generateKeys(3888);

    base::Bytes stress_msg(rsa.second.maxEncryptSize());
    auto enc_msg = rsa.second.encrypt(stress_msg);
    auto dec_msg = rsa.first.decrypt(enc_msg);
    BOOST_CHECK(stress_msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(Rsa_constructor_random_keys)
{
    auto rsa1 = base::generateKeys(3422);
    auto rsa2 = base::generateKeys(1898);

    base::Bytes msg{"RSA&Cons1r5tor"};
    auto enc_msg1 = rsa1.first.encrypt(msg);
    auto enc_msg2 = rsa2.first.encrypt(msg);
    BOOST_CHECK(enc_msg1 != enc_msg2);

    enc_msg1 = rsa1.second.encrypt(msg);
    enc_msg2 = rsa2.second.encrypt(msg);
    BOOST_CHECK(enc_msg1 != enc_msg2);
}

BOOST_AUTO_TEST_CASE(Rsa_serialization_constructor)
{
    auto rsa = base::generateKeys(3456);
    base::RsaPublicKey pub_key(rsa.first.toBytes());
    base::RsaPrivateKey priv_key(rsa.second.toBytes());

    base::Bytes msg{"Rs@_ser1al7ze construc"};
    auto enc_msg1 = rsa.first.encrypt(msg);
    auto enc_msg2 = pub_key.encrypt(msg);

    auto dec_msg1 = rsa.second.decrypt(enc_msg1);
    auto dec_msg2 = priv_key.decrypt(enc_msg2);
    BOOST_CHECK((msg == dec_msg1) && (msg == dec_msg2));
}


BOOST_AUTO_TEST_CASE(RsaPubKey_constructor_from_file_save_in_file)
{
    auto [pub_rsa, priv_rsa] = base::generateKeys(2012);
    base::RsaPublicKey pub_rsa2(pub_rsa);
    auto [pub_rsa3, priv_rsa3] = base::generateKeys(2344);
    pub_rsa3 = pub_rsa;

    BOOST_CHECK(pub_rsa.toBytes() == pub_rsa2.toBytes());
    BOOST_CHECK(pub_rsa.toBytes() == pub_rsa3.toBytes());
    BOOST_CHECK(pub_rsa3.toBytes() == pub_rsa2.toBytes());

    base::Bytes msg("RSa_FI1E_TES!");
    auto enc_msg = priv_rsa.encrypt(msg);

    BOOST_CHECK(pub_rsa.decrypt(enc_msg) == msg);
    BOOST_CHECK(pub_rsa2.decrypt(enc_msg) == msg);
    BOOST_CHECK(pub_rsa3.decrypt(enc_msg) == msg);
}


BOOST_AUTO_TEST_CASE(Rsa_constructor_from_file_save_in_file)
{
    auto [pub_rsa, priv_rsa] = base::generateKeys(3738);
    std::filesystem::path private_key_path{"ssh/rsa.priv"};
    std::filesystem::path public_key_path{"ssh/rsa.pub"};

    priv_rsa.save(private_key_path);
    pub_rsa.save(public_key_path);

    auto priv_rsa2 = base::RsaPrivateKey::load(private_key_path);
    auto pub_rsa2 = base::RsaPublicKey::load(public_key_path);

    base::Bytes msg("RSa_FI1E_TES!");
    auto enc_msg1 = priv_rsa.encrypt(msg);
    auto enc_msg2 = priv_rsa2.encrypt(msg);
    auto dec_msg1 = pub_rsa.decrypt(enc_msg1);
    auto dec_msg2 = pub_rsa2.decrypt(enc_msg2);
    BOOST_CHECK((dec_msg1 == dec_msg2) && (dec_msg1 == msg));
    BOOST_CHECK(enc_msg1 == enc_msg2);

    enc_msg1 = pub_rsa.encrypt(msg);
    enc_msg2 = pub_rsa2.encrypt(msg);
    dec_msg1 = priv_rsa.decrypt(enc_msg1);
    dec_msg2 = priv_rsa2.decrypt(enc_msg2);
    BOOST_CHECK(dec_msg1 == dec_msg2);

    std::filesystem::remove(private_key_path);
    std::filesystem::remove(public_key_path);
}

BOOST_AUTO_TEST_CASE(RsaAes_constructor_encrypt_decrypt)
{
    auto rsa = base::generateKeys(2894);
    base::Bytes msg("f1rst RsaAes_tes!");
    auto enc_msg = rsa.first.encryptWithAes(msg);
    auto dec_msg = rsa.second.decryptWithAes(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(RsaAes_small_key_encrypt_decrypt)
{
    auto rsa = base::generateKeys(1024);
    base::Bytes msg("f1rst RsaAes_tes!");
    auto enc_msg = rsa.first.encryptWithAes(msg);
    auto dec_msg = rsa.second.decryptWithAes(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

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

    std::filesystem::path key_path{"test.aes"};
    target_key.save(key_path);

    auto deserialized_key = base::AesKey::read(key_path);
    auto decrypt_message = deserialized_key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(target_msg, decrypt_message);

    std::filesystem::remove(key_path);
}


BOOST_AUTO_TEST_CASE(secp256_sign_verify)
{
    auto [pub_key, priv_key] = base::generateSecp256Keys();
    auto hash = base::Sha256::compute(base::Bytes("111"));
    auto signature = priv_key.sign(hash.getBytes());
    BOOST_CHECK(pub_key.verifySignature(signature, hash.getBytes()));
}


BOOST_AUTO_TEST_CASE(secp256_sign_verify_from_copy)
{
    auto [pub_key, priv_key] = base::generateSecp256Keys();
    auto pub_key2{pub_key};
    auto priv_key2{std::move(priv_key)};
    auto hash = base::Sha256::compute(base::Bytes("222"));

    auto signature = priv_key2.sign(hash.getBytes());
    BOOST_CHECK(pub_key2.verifySignature(signature, hash.getBytes()));
}


BOOST_AUTO_TEST_CASE(secp256_sign_verify_from_bytes_copy)
{
    auto [pub_key, priv_key] = base::generateSecp256Keys();
    base::Secp256PublicKey pub_key2{pub_key.getBytes()};
    base::Secp256PrivateKey priv_key2{priv_key.getBytes()};
    auto hash = base::Sha256::compute(base::Bytes("333"));

    auto signature1 = priv_key.sign(hash.getBytes());
    auto signature2 = priv_key2.sign(hash.getBytes());

    BOOST_CHECK(pub_key2.verifySignature(signature1, hash.getBytes()));
    BOOST_CHECK(pub_key2.verifySignature(signature2, hash.getBytes()));

    BOOST_CHECK(pub_key.verifySignature(signature1, hash.getBytes()));
    BOOST_CHECK(pub_key.verifySignature(signature2, hash.getBytes()));
}


BOOST_AUTO_TEST_CASE(secp256_sign_verify_with_another_key)
{
    auto [pub_key1, priv_key1] = base::generateSecp256Keys();
    auto [pub_key2, priv_key2] = base::generateSecp256Keys();
    auto hash = base::Sha256::compute(base::Bytes("444"));

    auto signature1 = priv_key1.sign(hash.getBytes());
    auto signature2 = priv_key2.sign(hash.getBytes());

    BOOST_CHECK(pub_key1.verifySignature(signature1, hash.getBytes()));
    BOOST_CHECK(pub_key2.verifySignature(signature2, hash.getBytes()));

    BOOST_CHECK(!pub_key2.verifySignature(signature1, hash.getBytes()));
    BOOST_CHECK(!pub_key1.verifySignature(signature2, hash.getBytes()));
}


BOOST_AUTO_TEST_CASE(secp256_save_load)
{
    auto [pub_key1, priv_key1] = base::generateSecp256Keys();
    std::filesystem::path private_key_path{"ssh/rsa.priv"};
    std::filesystem::path public_key_path{"ssh/rsa.pub"};

    priv_key1.save(private_key_path);
    pub_key1.save(public_key_path);

    auto priv_key2 = base::Secp256PrivateKey::load(private_key_path);
    auto pub_key2 = base::Secp256PublicKey::load(public_key_path);
    auto hash = base::Sha256::compute(base::Bytes("555"));

    auto signature1 = priv_key1.sign(hash.getBytes());
    auto signature2 = priv_key2.sign(hash.getBytes());

    BOOST_CHECK(pub_key2.verifySignature(signature1, hash.getBytes()));
    BOOST_CHECK(pub_key2.verifySignature(signature2, hash.getBytes()));

    BOOST_CHECK(pub_key1.verifySignature(signature1, hash.getBytes()));
    BOOST_CHECK(pub_key1.verifySignature(signature2, hash.getBytes()));

    std::filesystem::remove(private_key_path);
    std::filesystem::remove(public_key_path);
}

BOOST_AUTO_TEST_CASE(secp256_serialization)
{
    auto [pub_key1, priv_key1] = base::generateSecp256Keys();

    base::SerializationOArchive oa;
    oa.serialize(pub_key1);
    oa.serialize(priv_key1);

    base::SerializationIArchive ia(oa.getBytes());

    auto pub_key2 = ia.deserialize<base::Secp256PublicKey>();
    auto priv_key2 = ia.deserialize<base::Secp256PrivateKey>();
    auto hash = base::Sha256::compute(base::Bytes("667"));

    auto signature1 = priv_key1.sign(hash.getBytes());
    auto signature2 = priv_key2.sign(hash.getBytes());

    BOOST_CHECK(pub_key2.verifySignature(signature1, hash.getBytes()));
    BOOST_CHECK(pub_key2.verifySignature(signature2, hash.getBytes()));

    BOOST_CHECK(pub_key1.verifySignature(signature1, hash.getBytes()));
    BOOST_CHECK(pub_key1.verifySignature(signature2, hash.getBytes()));
}