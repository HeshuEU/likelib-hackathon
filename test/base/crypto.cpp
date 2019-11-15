#include <boost/test/unit_test.hpp>

#include <openssl/evp.h>

#include <base/crypto.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE(Rsa_constructor_encrypt_decrypt_check)
{
    base::Rsa rsa1(3788);
    base::Rsa rsa2(1675);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = rsa1.privateEncrypt(msg);
    auto dec_msg = rsa1.publicDecrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);

    enc_msg = rsa1.publicEncrypt(msg);
    dec_msg = rsa1.privateDecrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);

    base::Bytes stress_msg1(rsa1.maxPrivateEncryptSize());
    enc_msg = rsa1.privateEncrypt(stress_msg1);
    dec_msg = rsa1.publicDecrypt(enc_msg);
    BOOST_CHECK(stress_msg1 == dec_msg);

    base::Bytes stress_msg2(rsa1.maxPublicEncryptSize());
    enc_msg = rsa1.publicEncrypt(stress_msg2);
    dec_msg = rsa1.privateDecrypt(enc_msg);
    BOOST_CHECK(stress_msg2 == dec_msg);

    auto enc_msg1 = rsa1.privateEncrypt(msg);
    auto enc_msg2 = rsa2.privateEncrypt(msg);
    BOOST_CHECK(enc_msg1 != enc_msg2);

    enc_msg1 = rsa1.publicEncrypt(msg);
    enc_msg2 = rsa2.publicEncrypt(msg);
    BOOST_CHECK(enc_msg1 != enc_msg2);
}

BOOST_AUTO_TEST_CASE(Rsa_constructor_from_file_and_save_in_file)
{
    base::Rsa rsa1(3738);
    rsa1.save("public", "private");
    base::Rsa rsa2("public", "private");

    base::Bytes msg("RSa_FI1E_TES!");
    auto enc_msg1 = rsa1.privateEncrypt(msg);
    auto enc_msg2 = rsa2.privateEncrypt(msg);
    auto dec_msg1 = rsa1.publicDecrypt(enc_msg1);
    auto dec_msg2 = rsa2.publicDecrypt(enc_msg2);
    BOOST_CHECK((dec_msg1 == dec_msg2) && (dec_msg1 == msg));
    BOOST_CHECK(enc_msg1 == enc_msg2);
}