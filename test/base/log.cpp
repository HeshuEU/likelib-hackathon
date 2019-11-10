#include <boost/test/unit_test.hpp>

#include "base/log.hpp"

#include <iostream>

BOOST_AUTO_TEST_CASE(log_plain_usage)
{
    std::cout << "Note: expected enable file log output" << std::endl;
    base::initLog(base::LogLevel::ERROR);
    LOG_TRACE << "test message for trace log fn";
    LOG_DEBUG << "test message for debug log fn";
    LOG_INFO << "test message for info log fn";
    LOG_WARNING << "test message for warning log fn";
    LOG_ERROR << "test message for error log fn";
    LOG_FATAL << "test message for fatal log fn";
}


BOOST_AUTO_TEST_CASE(log_init_file_and_terminal)
{
    std::cout << "Note: expected enable file and terminal log outputs" << std::endl;
    base::initLog(base::LogLevel::ERROR, base::Sink::FILE | base::Sink::STDOUT);
    LOG_TRACE << "test message for trace log fn";
    LOG_DEBUG << "test message for debug log fn";
    LOG_INFO << "test message for info log fn";
    LOG_WARNING << "test message for warning log fn";
    LOG_ERROR << "test message for error log fn";
    LOG_FATAL << "test message for fatal log fn";
}


BOOST_AUTO_TEST_CASE(log_init_terminal)
{
    std::cout << "Note: expected enable terminal log output" << std::endl;
    base::initLog(base::LogLevel::ERROR, base::Sink::STDOUT);
    LOG_TRACE << "test message for trace log fn";
    LOG_DEBUG << "test message for debug log fn";
    LOG_INFO << "test message for info log fn";
    LOG_WARNING << "test message for warning log fn";
    LOG_ERROR << "test message for error log fn";
    LOG_FATAL << "test message for fatal log fn";
}


BOOST_AUTO_TEST_CASE(log_disable)
{
    std::cout << "Note: expected disable log outputs" << std::endl;
    base::initLog(base::LogLevel::ERROR, base::Sink::DISABLE);
    LOG_TRACE << "test message for trace log fn";
    LOG_DEBUG << "test message for debug log fn";
    LOG_INFO << "test message for info log fn";
    LOG_WARNING << "test message for warning log fn";
    LOG_ERROR << "test message for error log fn";
    LOG_FATAL << "test message for fatal log fn";
}