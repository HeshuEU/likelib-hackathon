#include <boost/test/unit_test.hpp>

#include "base/error.hpp"

BOOST_AUTO_TEST_CASE(error_code_to_string)
{
    BOOST_CHECK_EQUAL(base::EnumToString(base::StatusCode::FILE_NOT_FOUND), "FILE_NOT_FOUND");
    BOOST_CHECK_EQUAL(base::EnumToString(base::StatusCode::INVALID_ARGUMENT), "INVALID_ARGUMENT");
    BOOST_CHECK_EQUAL(base::EnumToString(static_cast<base::StatusCode>(-42)), nullptr);
}
