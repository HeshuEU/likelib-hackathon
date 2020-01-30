#pragma once

#include <boost/current_function.hpp>
#include <boost/log/trivial.hpp>

#include <cstddef>
#include <thread>

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


namespace Sink
{
    static constexpr const std::size_t DISABLE = 0x0;
    static constexpr const std::size_t STDOUT = 0x1;
    static constexpr const std::size_t FILE = 0x2;
} // namespace Sink


void initLog(LogLevel logLevel = LogLevel::ALL, std::size_t mode = Sink::FILE);

void dumpDebuggingInfo();

} // namespace base

#define LOG_TRACE BOOST_LOG_TRIVIAL(trace) << std::this_thread::get_id() << ' '
#define LOG_DEBUG BOOST_LOG_TRIVIAL(debug) << std::this_thread::get_id() << ' '
#define LOG_INFO BOOST_LOG_TRIVIAL(info) << std::this_thread::get_id() << ' '
#define LOG_WARNING BOOST_LOG_TRIVIAL(warning) << std::this_thread::get_id() << ' '
#define LOG_ERROR BOOST_LOG_TRIVIAL(error) << std::this_thread::get_id() << ' '
#define LOG_FATAL BOOST_LOG_TRIVIAL(fatal)

#define LOG_CURRENT_FUNCTION LOG_DEBUG << BOOST_CURRENT_FUNCTION << ' '
