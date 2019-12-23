#include <boost/test/unit_test.hpp>

#include "base/timer.hpp"

#include <thread>

constexpr std::int64_t milli_fault = 3;

BOOST_AUTO_TEST_CASE(time_usage1)
{
    base::Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    auto milli = timer.elapsedMillis();
    BOOST_CHECK(milli >= 30 && milli < (30 + milli_fault));
}


BOOST_AUTO_TEST_CASE(time_without_sleep)
{
    base::Timer timer;
    timer.start();

    auto milli = timer.elapsedMillis();
    BOOST_CHECK(milli <= 1);
}


BOOST_AUTO_TEST_CASE(time_usage2)
{
    base::Timer timer1;
    base::Timer timer2;
    timer1.start(); timer2.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(56));
    auto milli1 = timer1.elapsedMillis();
    auto milli2 = timer2.elapsedMillis();
    BOOST_CHECK(abs(milli1 - milli2) < milli_fault);
}


BOOST_AUTO_TEST_CASE(time_constructor_copy)
{
    base::Timer timer1;
    timer1.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(49));
    base::Timer timer2 = timer1;
    auto milli1 = timer1.elapsedMillis();
    auto milli2 = timer2.elapsedMillis();
    BOOST_CHECK(abs(milli1 - milli2) < milli_fault);
}


BOOST_AUTO_TEST_CASE(time_constructor_move)
{
    base::Timer timer1;
    timer1.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(49));
    base::Timer timer2 = std::move(timer1);
    auto milli1 = timer1.elapsedMillis();
    auto milli2 = timer2.elapsedMillis();
    BOOST_CHECK(abs(milli1 - milli2) < milli_fault);
}


BOOST_AUTO_TEST_CASE(time_elapsed_seconds)
{
    base::Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(149));
    BOOST_CHECK(abs(timer.elapsedSeconds() - 0.15) < 0.01);
}