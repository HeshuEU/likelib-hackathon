#pragma once

#include <boost/log/trivial.hpp>

namespace base
{

enum class LogLevel
{
    ALL,
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

void initLog(LogLevel logLevel = LogLevel::ALL);
} // namespace base

#define LOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define LOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define LOG_INFO BOOST_LOG_TRIVIAL(info)
#define LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define LOG_ERROR BOOST_LOG_TRIVIAL(error)
#define LOG_FATAL BOOST_LOG_TRIVIAL(fatal)
