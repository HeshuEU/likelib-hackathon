#include <boost/test/unit_test.hpp>

#include <base/crypto.hpp>

BOOST_AUTO_TEST_CASE(Rsa_constructor_encrypt_decrypt_check)
{
    base::Rsa rsa1(3201);
    base::Rsa rsa2(1675);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = rsa1.private_encrypt(msg);
    auto dec_msg = rsa1.public_decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);

    enc_msg = rsa1.public_encrypt(msg);
    dec_msg = rsa1.private_decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);

    base::Bytes stress_msg(rsa1.size());
    BOOST_CHECK(rsa1.size());

    auto enc_msg1 = rsa1.private_encrypt(msg);
    auto enc_msg2 = rsa2.private_encrypt(msg);
    BOOST_CHECK(enc_msg1 != enc_msg2);

    enc_msg1 = rsa1.public_encrypt(msg);
    enc_msg2 = rsa2.public_encrypt(msg);
    BOOST_CHECK(enc_msg1 != enc_msg2);
}

BOOST_AUTO_TEST_CASE(Rsa_constructor_from_file_and_save_in_file)
{
    base::Rsa rsa1(3738);
    rsa1.save("public", "private");
    base::Rsa rsa2("public", "private");

    base::Bytes msg("RSa_FI1E_TES!");
    auto enc_msg1 = rsa1.private_encrypt(msg);
    auto enc_msg2 = rsa2.private_encrypt(msg);
    auto dec_msg1 = rsa1.public_decrypt(enc_msg1);
    auto dec_msg2 = rsa2.public_decrypt(enc_msg2);
    BOOST_CHECK((dec_msg1 == dec_msg2) && (dec_msg1 == msg));
    BOOST_CHECK(enc_msg1 == enc_msg2);
}