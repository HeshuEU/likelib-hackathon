#include <boost/test/unit_test.hpp>

#include "base/time.hpp"

BOOST_AUTO_TEST_CASE(time_default_constructor)
{
    base::Time time1;
    base::Time time2;
    BOOST_CHECK(time1 == time2);
    BOOST_CHECK(time1.getSecondsInEpoch() == 0);
    BOOST_CHECK(time2.getSecondsInEpoch() == 0);
}


BOOST_AUTO_TEST_CASE(time_constructor_from_std)
{
    base::Time time1{base::Time::now()};
    auto time2 = base::Time::fromTimePoint(std::chrono::system_clock::now());
    BOOST_CHECK(time1.getSecondsInEpoch() == time2.getSecondsInEpoch());
}


BOOST_AUTO_TEST_CASE(time_operator_equal)
{
    base::Time time1{base::Time::now()};
    base::Time time2;
    BOOST_CHECK(time1 != time2);
    time2 = time1;
    BOOST_CHECK(time1 == time2);
    time2 = base::Time::now();
    BOOST_CHECK(time1.getSecondsInEpoch() == time2.getSecondsInEpoch());
}


BOOST_AUTO_TEST_CASE(time_from_seconds)
{
    base::Time time1(base::Time::now());
    auto seconds = time1.getSecondsInEpoch();
    base::Time time2(base::Time::fromSeconds(seconds));
    BOOST_CHECK(time1 == time2);
    BOOST_CHECK(time1.getSecondsInEpoch() == time2.getSecondsInEpoch());
}


BOOST_AUTO_TEST_CASE(time_serialization1)
{
    base::Time time1;
    base::SerializationOArchive oa;
    oa << time1;
    base::Time time2;
    base::SerializationIArchive ia(oa.getBytes());
    ia >> time2;
    BOOST_CHECK(time1 == time2);
}


BOOST_AUTO_TEST_CASE(time_serialization2)
{
    base::Time time1(base::Time::now());
    base::SerializationOArchive oa;
    oa << time1;
    base::Time time2;
    base::SerializationIArchive ia(oa.getBytes());
    ia >> time2;
    BOOST_CHECK(time1 == time2);
}


BOOST_AUTO_TEST_CASE(time_seconds_serialization)
{
    auto now{base::Time::now()};
    auto time_1_serialized{now.getSecondsInEpoch()};
    auto now_2{base::Time::fromSeconds(time_1_serialized)};
    auto time_2_serialized{now_2.getSecondsInEpoch()};
    BOOST_CHECK_EQUAL(time_1_serialized, time_2_serialized);
}
