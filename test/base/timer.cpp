#include <boost/test/unit_test.hpp>

#include "base/timer.hpp"

#include <thread>
#include <iostream>

constexpr std::int64_t milli_fault = 3;

BOOST_AUTO_TEST_CASE(time_usage1)
{
    base::Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto milli = timer.elapsedMillis();
    BOOST_CHECK(milli >= 100 && milli < (100 + milli_fault));
}


BOOST_AUTO_TEST_CASE(time_usage2)
{
    base::Timer timer1;
    base::Timer timer2;
    timer1.start(); timer2.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(213));
    auto milli1 = timer1.elapsedMillis();
    auto milli2 = timer2.elapsedMillis();
    BOOST_CHECK(abs(milli1 - milli2) < milli_fault);
}


BOOST_AUTO_TEST_CASE(time_constructor_copy)
{
    base::Timer timer1;
    timer1.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(149));
    base::Timer timer2 = timer1;
    auto milli1 = timer1.elapsedMillis();
    auto milli2 = timer2.elapsedMillis();
    BOOST_CHECK(abs(milli1 - milli2) < milli_fault);
}


BOOST_AUTO_TEST_CASE(time_elapsed_seconds)
{
    base::Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1149));
    BOOST_CHECK(abs(timer.elapsedSeconds() - 1.15) < 0.03);
}