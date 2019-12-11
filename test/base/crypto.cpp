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

BOOST_AUTO_TEST_CASE(Rsa_constructor_from_file_save_in_file)
{
    auto [pub_rsa, priv_rsa] = base::generateKeys(3738);
    priv_rsa.save("private");
    pub_rsa.save("public");
    base::RsaPrivateKey priv_rsa2("private");
    base::RsaPublicKey pub_rsa2("public");

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

    std::filesystem::remove("private");
    std::filesystem::remove("public");
}

BOOST_AUTO_TEST_CASE(RsaAes_constructor_encrypt_decrypt)
{
    auto rsa = base::generateKeys(2894);
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
    base::Bytes msg("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
    base::AesKey key(base::AesKey::KeyType::K128BIT);
    auto encrypted_data = key.encrypt(msg);
    auto decrypt_target = key.decrypt(encrypted_data);
    BOOST_CHECK_EQUAL(msg.toString(), decrypt_target.toString());
}

BOOST_AUTO_TEST_CASE(aes_encrypt_decrypt_128bit_by_serialized_key)
{
    base::Bytes msg("dfjbvalgecnhq=ygrbn3f5xgvidytnwucgfim2yx139sv7yx");
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
