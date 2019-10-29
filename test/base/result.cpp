#include <boost/test/unit_test.hpp>

#include "base/result.hpp"

namespace
{

base::Result<int> someFunction(int x)
{
    if(x % 2 == 0) {
        return x / 2;
    }
    else {
        return base::Error{base::ErrorCode::INVALID_PARAMETER, "is not even"};
    }
}

}


BOOST_AUTO_TEST_CASE(result_usage)
{
    auto s1 = someFunction(0);
    BOOST_CHECK(s1);
    BOOST_CHECK(s1.isOk());
    BOOST_CHECK(!s1.isError());

    auto s2 = someFunction(1);
    BOOST_CHECK(!s2);
    BOOST_CHECK(s2.isError());

    if(auto r = someFunction(28)) {
        // do some job
        int y = r;
        BOOST_CHECK_EQUAL(y, 14);
    }
    else /* if failed */ {
        BOOST_FAIL("not supposed to go here");
    }

    int v = someFunction(28).getValue() * 2;
    BOOST_CHECK_EQUAL(v, 28);
}