#include "log.hpp"

#include <boost/log/sources/record_ostream.hpp>

#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <boost/log/support/date_time.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>


namespace base
{

static constexpr const char* LOG_FILE_FORMAT = "app_%m-%d-%Y_%H:%M.log";
static constexpr const char* LOG_FOLDER = "logs";
static constexpr const int LOG_FILE_MAX_SIZE = 6 * 1024 * 1024;
static constexpr const int LOG_FILE_MIN_SPACE = 100 * 1024 * 1024;
static constexpr const int MAX_LOG_FILE_COUNT = 512;

void setLogLevel(LogLevel logLevel)
{
    switch(logLevel) {
        case LogLevel::ALL:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
            break;
        case LogLevel::DEBUG:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
            break;
        case LogLevel::INFO:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
            break;
        case LogLevel::WARNING:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
            break;
        case LogLevel::ERROR:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::error);
            break;
    }
}

void formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
{
    strm << boost::posix_time::second_clock::local_time();
    strm << " |" << rec[boost::log::trivial::severity] << "| " << rec[boost::log::expressions::smessage];
}

void setLogFileSettings()
{
    using textFileSink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>;

    boost::shared_ptr<textFileSink> sink(new textFileSink(boost::log::keywords::file_name = LOG_FILE_FORMAT));

    sink->locked_backend()->set_file_collector(boost::log::sinks::file::make_collector(
        boost::log::keywords::target = LOG_FOLDER, boost::log::keywords::max_size = LOG_FILE_MAX_SIZE,
        boost::log::keywords::min_free_space = LOG_FILE_MIN_SPACE,
        boost::log::keywords::max_files = MAX_LOG_FILE_COUNT));

    sink->set_formatter(&formatter);

    boost::log::core::get()->add_sink(sink);
}

void setTerminalSettings()
{
    using text_sink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;

    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

    sink->set_formatter(&formatter);

    boost::log::core::get()->add_sink(sink);
}

void initLog(LogLevel logLevel)
{
    setLogLevel(logLevel);
    setLogFileSettings();
    setTerminalSettings();
    boost::log::add_common_attributes();
}

} // namespace base
