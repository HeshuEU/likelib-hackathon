#include <boost/test/unit_test.hpp>

#include "base/bytes.hpp"
#include "base/hash.hpp"


BOOST_AUTO_TEST_CASE(sha_256_hash)
{
    auto sha256_1 =
        base::Sha256::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    BOOST_CHECK_EQUAL(sha256_1.toHex(), "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846");
    BOOST_CHECK_EQUAL(sha256_1.getBytes().toHex(), "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846");

    auto sha256_2 = base::Sha256::compute(base::Bytes("likelib.2"));
    BOOST_CHECK_EQUAL(sha256_2.toHex(), "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5");
    BOOST_CHECK_EQUAL(sha256_2.getBytes().toHex(), "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5");

    auto sha256_3 = base::Sha256::compute(base::Bytes("it's third test"));
    BOOST_CHECK_EQUAL(sha256_3.toHex(), "2431f272555362f2d9ee255ec2ea24dffa371a03137699cf9d0b96e988346421");
    BOOST_CHECK_EQUAL(sha256_3.getBytes().toHex(), "2431f272555362f2d9ee255ec2ea24dffa371a03137699cf9d0b96e988346421");

    auto sha256_4 = base::Sha256::compute(base::Bytes(""));
    BOOST_CHECK_EQUAL(sha256_4.toHex(), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    BOOST_CHECK_EQUAL(sha256_4.getBytes().toHex(), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}


BOOST_AUTO_TEST_CASE(sha_1_hash)
{
    auto sha1_1 = base::Sha1::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    BOOST_CHECK_EQUAL(sha1_1.toHex(), "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08");
    BOOST_CHECK_EQUAL(sha1_1.getBytes().toHex(), "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08");

    auto sha1_2 = base::Sha1::compute(base::Bytes("likelib.2"));
    BOOST_CHECK_EQUAL(sha1_2.toHex(), "ee2f5885c39b865f83e5f91dd94ce466f3be371d");
    BOOST_CHECK_EQUAL(sha1_2.getBytes().toHex(), "ee2f5885c39b865f83e5f91dd94ce466f3be371d");

    auto sha1_3 = base::Sha1::compute(base::Bytes("it's third test"));
    BOOST_CHECK_EQUAL(sha1_3.toHex(), "3f68e91144cc3d272df2950c0676918980a35d01");
    BOOST_CHECK_EQUAL(sha1_3.getBytes().toHex(), "3f68e91144cc3d272df2950c0676918980a35d01");

    auto sha1_4 = base::Sha1::compute(base::Bytes(""));
    BOOST_CHECK_EQUAL(sha1_4.toHex(), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    BOOST_CHECK_EQUAL(sha1_4.getBytes().toHex(), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
}
