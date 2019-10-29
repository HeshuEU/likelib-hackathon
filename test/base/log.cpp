#include <boost/test/unit_test.hpp>

#include "base/log.hpp"

BOOST_AUTO_TEST_CASE(log_plain_usage)
{
    base::initLog(base::LogLevel::ERROR);
    LOG_TRACE << "test message for trace log fn";
    LOG_DEBUG << "test message for debug log fn";
    LOG_INFO << "test message for info log fn";
    LOG_WARNING << "test message for warning log fn";
    LOG_ERROR << "test message for error log fn";
    LOG_FATAL << "test message for fatal log fn";
}