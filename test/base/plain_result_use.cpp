#include <boost/test/unit_test.hpp>

#include "base/result.hpp"
#include "base/status.hpp"

#include <string>
#include <cstddef>

// emulate fail call of process if isTrue
int plain_system_call(bool isTrue)
{
    if(isTrue) {
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}


base::Status setUpSystemLibFailed()
{
    if(plain_system_call(false) != EXIT_SUCCESS) {
        return base::Status::Error("Fail in process call");
    }
    return base::Status::Ok();
}


base::Result<std::string> checkEmptyAndReturnHello(const std::string& input)
{
    if(input.empty()) {
        return base::Error("String is empty");
    }
    return std::string{"Hello, "} + input;
}


BOOST_AUTO_TEST_CASE(plain_status_use)
{
    BOOST_CHECK(!setUpSystemLibFailed());
}


BOOST_AUTO_TEST_CASE(plain_result_use)
{
    auto res = checkEmptyAndReturnHello("Vasia");
    BOOST_CHECK(res);
    res = checkEmptyAndReturnHello("");
    BOOST_CHECK(!res);
}