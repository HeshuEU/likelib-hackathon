#include <boost/test/unit_test.hpp>

#include "base/asymmetric_crypto.hpp"

BOOST_AUTO_TEST_CASE(plain_asymetric_key_gen)
{
    auto key_pair = base::generate(2048);

}
