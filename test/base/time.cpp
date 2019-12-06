#include <boost/test/unit_test.hpp>

#include "base/time.hpp"

BOOST_AUTO_TEST_CASE(time_seconds_serialization)
{
    auto now =  base::Time::now();
    auto time_1_serialized = now.seconds();
    auto now_2 = base::Time::fromSeconds(time_1_serialized);
    auto time_2_serialized = now_2.seconds();

    BOOST_CHECK_EQUAL(time_1_serialized, time_2_serialized);
}