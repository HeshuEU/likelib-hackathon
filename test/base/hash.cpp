#include <boost/test/unit_test.hpp>

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include <iostream>


BOOST_AUTO_TEST_CASE(sha_256_hash)
{
    base::Bytes bytes1{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30};
    BOOST_CHECK_EQUAL(base::sha256(bytes1).toHex(), "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846");

    base::Bytes bytes2("likelib.2");
    BOOST_CHECK_EQUAL(base::sha256(bytes2).toHex(), "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5");

    //TODO: Not work now
    //base::Bytes bytes3("its third test");
    //BOOST_CHECK_EQUAL(base::sha256(bytes3).toHex(), "fa0b079f72dc4d385f65914b9fc49f6e6e3a226df93379fd00f7637e9f4e471f");
}


BOOST_AUTO_TEST_CASE(sha_1_hash)
{
    base::Bytes bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30};
    BOOST_CHECK_EQUAL(base::sha1(bytes).toHex(), "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08");
}
