#include <boost/test/unit_test.hpp>

#include "base/asymmetric_crypto.hpp"

BOOST_AUTO_TEST_CASE(plain_asymetric_key_gen)
{
    auto key_pair = base::generateRsaKeys(2048);

    base::Bytes msg{"RSA_CONSTRUCTOR_TEST"};
    auto enc_msg = key_pair.first.encrypt(msg);
    auto dec_msg = key_pair.second.decrypt(enc_msg);
    BOOST_CHECK(msg == dec_msg);
}
