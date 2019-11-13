#include <boost/test/unit_test.hpp>

#include <base/crypto.hpp>

BOOST_AUTO_TEST_CASE(Rsa_constructor_check)
{
    base::Rsa rsa;
    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    base::Bytes enc_msg = rsa.private_encrypt(msg);
    base::Bytes dec_msg = rsa.public_decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}
