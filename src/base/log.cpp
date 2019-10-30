#include "log.hpp"

#include "base/config.hpp"

#include <boost/log/sources/record_ostream.hpp>

#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <boost/log/support/date_time.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>


namespace
{

void setLogLevel(base::LogLevel logLevel)
{
    switch(logLevel) {
        case base::LogLevel::ALL:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
            break;
        case base::LogLevel::DEBUG:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
            break;
        case base::LogLevel::INFO:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
            break;
        case base::LogLevel::WARNING:
            boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
            break;
        case base::LogLevel::ERROR:
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

    boost::shared_ptr<textFileSink> sink(
        new textFileSink(boost::log::keywords::file_name = base::config::LOG_FILE_FORMAT));

    sink->locked_backend()->set_file_collector(
        boost::log::sinks::file::make_collector(boost::log::keywords::target = base::config::LOG_FOLDER,
                                                boost::log::keywords::max_size = base::config::LOG_FILE_MAX_SIZE,
                                                boost::log::keywords::min_free_space = base::config::LOG_FILE_MIN_SPACE,
                                                boost::log::keywords::max_files = base::config::MAX_LOG_FILE_COUNT));

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

void disableLogger()
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::fatal);
}

}

namespace base
{

void initLog(base::LogLevel logLevel, size_t mode)
{
    if(!mode) {
        std::cout << "WARNING: LOG OUTPUT IS DISABLED" << std::endl; // TODO: remove later
        disableLogger();
        return;
    }

    setLogLevel(logLevel);

    if(mode & base::TERMINAL) {
        setTerminalSettings();
        std::cout << "WARNING: TERMINAL LOG OUTPUT INITED" << std::endl; // TODO: remove later
    }
    if(mode & base::FILE) {
        setLogFileSettings();
        std::cout << "WARNING: FILE LOG OUTPUT INITED" << std::endl; // TODO: remove later
    }

    boost::log::add_common_attributes();
}

}
