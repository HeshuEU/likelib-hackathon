#include <boost/test/unit_test.hpp>

#include "base/asymmetric_crypto.hpp"
#include "base/error.hpp"

BOOST_AUTO_TEST_CASE(asymmetric_public_encrypt_private_decrypt)
{
    auto key_pair = base::rsa::generateKeys(2048);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.first.encrypt(msg);
    auto dec_msg = key_pair.second.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(asymmetric_private_encrypt_public_decrypt)
{
    auto key_pair = base::rsa::generateKeys(2048);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.second.encrypt(msg);
    auto dec_msg = key_pair.first.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(asymmetric_private_encrypt_private_decrypt)
{
    auto key_pair = base::rsa::generateKeys(2048);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.second.encrypt(msg);
    BOOST_CHECK_THROW(auto dec_msg = key_pair.second.decrypt(enc_msg), base::Error);
}

BOOST_AUTO_TEST_CASE(asymmetric_private_encrypt_public_decrypt_with_serialization)
{
    auto key_pair = base::rsa::generateKeys(2048);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.second.encrypt(msg);

    auto public_key_bytes = key_pair.first.toBytes();
    base::rsa::PublicKey deserialized_public_key(public_key_bytes);

    auto dec_msg = deserialized_public_key.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}