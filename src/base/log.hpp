#pragma once

#include <boost/current_function.hpp>
#include <boost/log/keywords/severity.hpp>
#include <boost/log/sources/features.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/threading_models.hpp>
#include <boost/log/trivial.hpp>

#include <cstddef>
#include <mutex>
#include <sstream>
#include <thread>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(logger, src::severity_logger_mt<logging::trivial::severity_level>)

namespace base
{

namespace Sink
{

static constexpr const std::size_t DISABLE = 0x0;
static constexpr const std::size_t STDOUT = 0x1;
static constexpr const std::size_t FILE = 0x2;

} // namespace Sink


void initLog(std::size_t mode = Sink::FILE);

void dumpDebuggingInfo();

void flushLog();

} // namespace base


#if defined(CONFIG_IS_DEBUG)
#define LOG_DEBUG BOOST_LOG_SEV(logger::get(), logging::trivial::debug)
#else

class NullBuffer
{};

template<typename T>
[[maybe_unused]] const NullBuffer& operator<<(const NullBuffer& nb, T&&)
{
    return nb;
}

#define LOG_DEBUG                                                                                                      \
    NullBuffer {}
#endif

#define LOG_TRACE BOOST_LOG_SEV(logger::get(), logging::trivial::trace) << BOOST_CURRENT_FUNCTION << ' '
#define LOG_INFO BOOST_LOG_SEV(logger::get(), logging::trivial::info)
#define LOG_WARNING BOOST_LOG_SEV(logger::get(), logging::trivial::warning)
#define LOG_ERROR BOOST_LOG_SEV(logger::get(), logging::trivial::error)
#define LOG_FATAL BOOST_LOG_SEV(logger::get(), logging::trivial::fatal)
