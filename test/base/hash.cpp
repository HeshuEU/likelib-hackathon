#include <boost/test/unit_test.hpp>

#include "base/bytes.hpp"
#include "base/hash.hpp"


BOOST_AUTO_TEST_CASE(sha_256_hash)
{
    base::Bytes bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30};
    // TODO: uncomment later
    // BOOST_CHECK_EQUAL(base::sha256(bytes).toHex(),
    // "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846");
}


BOOST_AUTO_TEST_CASE(sha_1_hash)
{
    base::Bytes bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30};
    // TODO: uncomment later
    // BOOST_CHECK_EQUAL(base::sha1(bytes).toHex(), "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08");
}
