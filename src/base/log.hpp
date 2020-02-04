#pragma once

#include <boost/current_function.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/features.hpp>
#include <boost/log/sources/threading_models.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

#include <cstddef>
#include <thread>
#include <sstream>
#include <mutex>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;

enum Severity
{
    TRACE,
    DEBUG,
    WARNING,
    INFO,
    ERROR,
    FATAL
};

// BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(logger, src::logger_mt)

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


class Logger
{
  public:
    template<typename T>
    void print(T&& obj) const
    {
        oss << obj;
    }

    ~Logger()
    {
        static std::recursive_mutex mut;
        mut.lock();
        BOOST_LOG_TRIVIAL(debug) << oss.str();
        mut.unlock();
    }

  private:
    mutable std::ostringstream oss;
};

template<typename T>
const Logger& operator<<(const Logger& logger, T&& obj)
{
    logger.print(obj);
    return logger;
}


//#define LOG_TRACE BOOST_LOG_SEV(logger::get(), Severity::INFO) << std::this_thread::get_id() << ' '
//#define LOG_DEBUG BOOST_LOG_SEV(logger::get(), Severity::DEBUG) << std::this_thread::get_id() << ' '
//#define LOG_INFO BOOST_LOG_SEV(logger::get(), Severity::INFO) << std::this_thread::get_id() << ' '
//#define LOG_WARNING BOOST_LOG_SEV(logger::get(), Severity::WARNING) << std::this_thread::get_id() << ' '
//#define LOG_ERROR BOOST_LOG_SEV(logger::get(), Severity::ERROR) << std::this_thread::get_id() << ' '
//#define LOG_FATAL BOOST_LOG_SEV(logger)

#define LOG_TRACE \
    Logger \
    {}
#define LOG_DEBUG \
    Logger \
    {}
#define LOG_INFO \
    Logger \
    {}
#define LOG_WARNING \
    Logger \
    {}
#define LOG_ERROR \
    Logger \
    {}
#define LOG_FATAL BOOST_LOG(logger)

#define LOG_CURRENT_FUNCTION LOG_DEBUG << BOOST_CURRENT_FUNCTION << ' '
