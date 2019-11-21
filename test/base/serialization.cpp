#include <boost/test/unit_test.hpp>

#include "base/serialization.hpp"

BOOST_AUTO_TEST_CASE(serialization_sanity_check1)
{
    base::SerializationOArchive oa;
    oa << base::Byte{0x12} << -35 << -7ll;

    base::SerializationIArchive ia(oa.getBytes());
    base::Byte a;
    int b;
    long long c;
    ia >> a >> b >> c;
    BOOST_CHECK_EQUAL(a, 0x12);
    BOOST_CHECK_EQUAL(b,-35);
    BOOST_CHECK_EQUAL(c, -7);
}


BOOST_AUTO_TEST_CASE(serialization_sanity_check2)
{
    const std::size_t aa = 0x35;
    base::Bytes bb{0x1, 0x3, 0x5, 0x7, 0x15};
    const std::vector<std::string> cc{"a", "b", "c"};

    base::SerializationOArchive oa;
    oa << aa << bb << cc;

    base::SerializationIArchive ia(oa.getBytes());

    std::size_t a;
    ia >> a;
    BOOST_CHECK_EQUAL(a, aa);

    base::Bytes b;
    ia >> b;
    BOOST_CHECK(b == bb);

    std::vector<std::string> c;
    ia >> c;
    BOOST_CHECK(c == cc);
}
