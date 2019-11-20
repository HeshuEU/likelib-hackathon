#include <boost/test/unit_test.hpp>

#include "base/serialization.hpp"

BOOST_AUTO_TEST_CASE(serialization_sanity_check)
{
    base::SerializationIArchive ia;
    ia << base::Byte{0x12} << -35 << -7ll;

    base::SerializationOArchive oa(ia.getBytes());
    base::Byte a;
    int b;
    long long c;
    oa >> a >> b >> c;
    BOOST_CHECK_EQUAL(a, 0x12);
    BOOST_CHECK_EQUAL(b,-35);
    BOOST_CHECK_EQUAL(c, -7);
}
