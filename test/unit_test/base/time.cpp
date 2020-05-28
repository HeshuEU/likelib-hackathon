#include <boost/test/unit_test.hpp>

#include "base/time.hpp"

BOOST_AUTO_TEST_CASE(time_default_constructor)
{
    base::Time time1;
    base::Time time2;
    BOOST_CHECK(time1 == time2);
    BOOST_CHECK(time1.getSeconds() == 0);
    BOOST_CHECK(time2.getSeconds() == 0);
}


BOOST_AUTO_TEST_CASE(time_from_TimePoint1)
{
    base::Time time1{ base::Time::now() };
    auto time2 = base::Time(std::chrono::system_clock::now());
    BOOST_CHECK(time1.getSeconds() == time2.getSeconds());
    BOOST_CHECK(time1.toTimePoint() == time2.toTimePoint());
    BOOST_CHECK(time1 == time2);
}


BOOST_AUTO_TEST_CASE(time_from_TimePoint2)
{
    base::Time time1{ base::Time::now() };
    base::Time time2{ base::Time(time1.toTimePoint()) };
    BOOST_CHECK(time1.getSeconds() == time2.getSeconds());
    BOOST_CHECK(time1.toTimePoint() == time2.toTimePoint());
    BOOST_CHECK(time1 == time2);
}


BOOST_AUTO_TEST_CASE(time_operator_equal)
{
    base::Time time1{ base::Time::now() };
    base::Time time2;
    BOOST_CHECK(time1 != time2);
    time2 = time1;
    BOOST_CHECK(time1 == time2);
    time2 = base::Time::now();
    BOOST_CHECK(time1.getSeconds() == time2.getSeconds());
}


BOOST_AUTO_TEST_CASE(time_from_seconds)
{
    base::Time time1(base::Time::now());
    auto seconds = time1.getSeconds();
    base::Time time2(seconds);
    BOOST_CHECK(time1 == time2);
    BOOST_CHECK(time1.getSeconds() == time2.getSeconds());
}


BOOST_AUTO_TEST_CASE(time_serialization1)
{
    base::Time time1;
    base::SerializationOArchive oa;
    oa.serialize(time1);
    base::SerializationIArchive ia(oa.getBytes());
    auto time2 = ia.deserialize<base::Time>();
    BOOST_CHECK(time1 == time2);
}


BOOST_AUTO_TEST_CASE(time_serialization2)
{
    base::Time time1(base::Time::now());
    base::SerializationOArchive oa;
    oa.serialize(time1);
    base::SerializationIArchive ia(oa.getBytes());
    auto time2 = ia.deserialize<base::Time>();
    BOOST_CHECK(time1 == time2);
}


BOOST_AUTO_TEST_CASE(time_seconds_serialization)
{
    auto now{ base::Time::now() };
    auto time_1_serialized{ now.getSeconds() };
    auto now_2{ base::Time(time_1_serialized) };
    auto time_2_serialized{ now_2.getSeconds() };
    BOOST_CHECK_EQUAL(time_1_serialized, time_2_serialized);
}


BOOST_AUTO_TEST_CASE(time_toBytes_from_Bytes)
{
    auto time1{ base::Time::now() };
    auto b = base::toBytes(time1);
    auto time2 = base::fromBytes<base::Time>(b);
    BOOST_CHECK(time1.getSeconds() == time2.getSeconds());
    BOOST_CHECK(time1.toTimePoint() == time2.toTimePoint());
}
