#include <boost/test/unit_test.hpp>

#include "base/asymmetric_crypto.hpp"
#include "base/error.hpp"

/*BOOST_AUTO_TEST_CASE(asymmetric_public_encrypt_private_decrypt)
{
    auto key_pair = base::rsa::generateKeys(2048);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.first.encrypt(msg);
    auto dec_msg = key_pair.second.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(asymmetric_private_encrypt_public_decrypt)
{
    auto key_pair = base::rsa::generateKeys(3568);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.second.encrypt(msg);
    auto dec_msg = key_pair.first.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(asymmetric_private_encrypt_private_decrypt)
{
    auto key_pair = base::rsa::generateKeys(5684);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.second.encrypt(msg);
    BOOST_CHECK_THROW(auto dec_msg = key_pair.second.decrypt(enc_msg), base::Error);
}

BOOST_AUTO_TEST_CASE(asymmetric_private_encrypt_public_decrypt_with_serialization)
{
    auto key_pair = base::rsa::generateKeys(4000);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.second.encrypt(msg);

    auto public_key_bytes = key_pair.first.toBytes();
    base::rsa::RsaPublicKey deserialized_public_key(public_key_bytes);

    auto dec_msg = deserialized_public_key.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}

BOOST_AUTO_TEST_CASE(asimmetric_encrypt_decrypt_with_aes)
{
    auto [pub_key, priv_key] = base::rsa::generateKeys(3072);

    base::Bytes msg{"fkldnvibbeucbsdfvcui"
                    "sadbfvuicaheiyfbvcbouvneg yb"
                    "gyncyegrvyuf gvrbuncreanfrcifnemdulhvyin35"
                    "gcmfa,oxmervhnutgy754tcnmx,"
                    "z5msnygtc5v7mtygs4nd2nftonvcmufgac ljvwbt"
                    "pygqin3cxg096mcgf5nsm2089ycpmfhv07n4m5v;kjfb jucgvjaf"
                    "jvbdubfdhbcufinag bcnrbxgmicnyvgb6nytgc53m8noy47mg5n4"
                    "rt83yfiudhg34btn3cby5cn8oxmqc3cnvbc34tn83toалротгшминтамт  мрауимамвыгми"
                    "оракумтпсьуквинтемьсч47ст7е27нч"};
    auto enc_msg = pub_key.encryptWithtAes(msg);
    auto dec_msg = priv_key.dectyptWithAes(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}*/