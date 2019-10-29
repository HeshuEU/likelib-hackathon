#include <boost/test/unit_test.hpp>

#include "base/error.hpp"

BOOST_AUTO_TEST_CASE(error_code_to_string)
{
    BOOST_CHECK_EQUAL(base::EnumToString(base::ErrorCode::FILE_NOT_FOUND), "FILE_NOT_FOUND");
    BOOST_CHECK_EQUAL(base::EnumToString(base::ErrorCode::INVALID_PARAMETER), "INVALID_PARAMETER");
    BOOST_CHECK_EQUAL(base::EnumToString(static_cast<base::ErrorCode>(-42)), nullptr);
}
