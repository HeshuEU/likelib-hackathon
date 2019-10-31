#include <boost/test/unit_test.hpp>

#include "base/result.hpp"
#include "base/status.hpp"

#include <string>

namespace
{

base::Result<int> someFunction(int x)
{
    if(x % 2 == 0) {
        return x / 2;
    }
    else {
        return base::Error{base::StatusCode::INVALID_ARGUMENT, "is not even"};
    }
}

} // namespace


BOOST_AUTO_TEST_CASE(result_usage1)
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


namespace
{
base::Result<std::string> buildUpString(bool gonna_fail)
{
    if(gonna_fail) {
        return base::Error(base::StatusCode::INVALID_ARGUMENT, "set to fail");
    }
    else {
        return "ab " + std::to_string(0x42) + " cd";
    }
}

base::Result<std::string> thisFunctionUsesBuildUpString(bool gonna_fail)
{
    if(auto result = buildUpString(gonna_fail)) {
        return result.getValue() + result.getValue();
    }
    else {
        return result;
    }
}

base::Status topMostFunction(bool gonna_fail)
{
    if(auto result = thisFunctionUsesBuildUpString(gonna_fail)) {
        return base::Status::Ok();
    }
    else {
        return base::Status::Error(base::StatusCode::INVALID_ARGUMENT);
    }
}
} // namespace


BOOST_AUTO_TEST_CASE(result_usage2)
{
    auto s1 = topMostFunction(true);
    BOOST_CHECK(!s1);
    BOOST_CHECK(topMostFunction(false));
}
