#include <boost/test/unit_test.hpp>

#include "base/status.hpp"


namespace
{

base::Status someFunction(int x)
{
    if(x % 2 == 0) {
        return base::Status::Ok();
    }
    else {
        return base::Status::Error({base::StatusCode::INVALID_ARGUMENT, "is not even"});
    }
}

} // namespace


BOOST_AUTO_TEST_CASE(status_usage)
{
    auto s1 = someFunction(0);
    BOOST_CHECK(s1);
    BOOST_CHECK(s1.isOk());
    BOOST_CHECK(!s1.isError());

    auto s2 = someFunction(1);
    BOOST_CHECK(!s2);
    BOOST_CHECK(s2.isError());

    base::Status s3 = s2;
    BOOST_CHECK(!s3);
    BOOST_CHECK(s3.isError());
}