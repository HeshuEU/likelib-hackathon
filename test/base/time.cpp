#include <boost/test/unit_test.hpp>

#include "base/time.hpp"

BOOST_AUTO_TEST_CASE(time_miliseconds_serialization)
{
    auto now =  base::Time::now();
    auto time_1_serialized = now.millisecondsInEpoch();
    auto now_2 = base::Time::fromMilliseconds(time_1_serialized);
    auto time_2_serialized = now_2.millisecondsInEpoch();

    BOOST_CHECK_EQUAL(time_1_serialized, time_2_serialized);
}

BOOST_AUTO_TEST_CASE(time_seconds_serialization)
{
    auto now =  base::Time::now();
    auto time_1_serialized = now.secondsInEpoch();
    auto now_2 = base::Time::fromSeconds(time_1_serialized);
    auto time_2_serialized = now_2.secondsInEpoch();

    BOOST_CHECK_EQUAL(time_1_serialized, time_2_serialized);
}