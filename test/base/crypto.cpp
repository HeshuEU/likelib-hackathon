#include <boost/test/unit_test.hpp>

#include <openssl/evp.h>

#include <base/crypto.hpp>

BOOST_AUTO_TEST_CASE(Rsa_constructor_encrypt_decrypt_check)
{
    auto rsa1 = base::generate(2048);
    auto rsa2 = base::generate(1675);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = rsa1.first.encrypt(msg);
    auto dec_msg = rsa1.second.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);

    enc_msg = rsa1.second.encrypt(msg);
    dec_msg = rsa1.first.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);

    base::Bytes stress_msg1(rsa1.first.maxEncryptSize());
    enc_msg = rsa1.first.encrypt(stress_msg1);
    dec_msg = rsa1.second.decrypt(enc_msg);
    BOOST_CHECK(stress_msg1 == dec_msg);

    base::Bytes stress_msg2(rsa1.second.maxEncryptSize());
    enc_msg = rsa1.second.encrypt(stress_msg2);
    dec_msg = rsa1.first.decrypt(enc_msg);
    BOOST_CHECK(stress_msg2 == dec_msg);

    auto enc_msg1 = rsa1.first.encrypt(msg);
    auto enc_msg2 = rsa2.first.encrypt(msg);
    BOOST_CHECK(enc_msg1 != enc_msg2);

    enc_msg1 = rsa1.second.encrypt(msg);
    enc_msg2 = rsa2.second.encrypt(msg);
    BOOST_CHECK(enc_msg1 != enc_msg2);
}


BOOST_AUTO_TEST_CASE(Rsa_constructor_from_file_and_save_in_file)
{
    auto rsa1 = base::generate(3738);
    rsa1.first.save("private");
    rsa1.second.save("public");
    base::PrivateRsaKey rsa_priv("private");
    base::PublicRsaKey rsa_pub("public");

    base::Bytes msg("RSa_FI1E_TES!");
    auto enc_msg1 = rsa1.first.encrypt(msg);
    auto enc_msg2 = rsa_priv.encrypt(msg);
    auto dec_msg1 = rsa1.second.decrypt(enc_msg1);
    auto dec_msg2 = rsa_pub.decrypt(enc_msg2);
    BOOST_CHECK((dec_msg1 == dec_msg2) && (dec_msg1 == msg));
    BOOST_CHECK(enc_msg1 == enc_msg2);
}


BOOST_AUTO_TEST_CASE(RsaAes_constructor_encrypt_decrypt)
{
    auto rsa = base::generate(2894);
    base::Bytes msg("f1rst RsaAes_tes!");
    auto enc_msg = rsa.second.encryptWithAes(msg);
    auto dec_msg = rsa.first.decryptWithAes(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}
