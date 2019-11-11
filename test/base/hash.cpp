#include <boost/test/unit_test.hpp>

#include "base/bytes.hpp"
#include "base/hash.hpp"


BOOST_AUTO_TEST_CASE(sha_256_hash)
{
    base::Bytes bytes1{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30};
    BOOST_CHECK_EQUAL(base::sha256(bytes1).toHex(), "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846");

    base::Bytes bytes2("likelib.2");
    BOOST_CHECK_EQUAL(base::sha256(bytes2).toHex(), "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5");

    base::Bytes bytes3("it's third test");
    BOOST_CHECK_EQUAL(base::sha256(bytes3).toHex(), "2431f272555362f2d9ee255ec2ea24dffa371a03137699cf9d0b96e988346421");

    base::Bytes bytes("");
    BOOST_CHECK_EQUAL(base::sha256(bytes).toHex(), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}


BOOST_AUTO_TEST_CASE(sha_1_hash)
{
    base::Bytes bytes1{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30};
    BOOST_CHECK_EQUAL(base::sha1(bytes1).toHex(), "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08");

    base::Bytes bytes2("likelib.2");
    BOOST_CHECK_EQUAL(base::sha1(bytes2).toHex(), "ee2f5885c39b865f83e5f91dd94ce466f3be371d");

    base::Bytes bytes3("it's third test");
    BOOST_CHECK_EQUAL(base::sha1(bytes3).toHex(), "3f68e91144cc3d272df2950c0676918980a35d01");

    base::Bytes bytes("");
    BOOST_CHECK_EQUAL(base::sha1(bytes).toHex(), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
}
