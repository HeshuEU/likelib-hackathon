#include "log.hpp"

#include "base/config.hpp"

#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/stacktrace.hpp>

#include <ctime>
#include <filesystem>

namespace
{

std::string dateAsString()
{
    std::time_t raw_time;
    std::tm* time_info;
    char buffer[80];
    std::time(&raw_time);
    time_info = std::localtime(&raw_time);
    std::strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", time_info);
    return buffer;
}

void clearLoggerSettings()
{
    boost::log::core::get()->remove_all_sinks();
}

void formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& stream)
{
    stream << dateAsString() << " | " << rec[boost::log::trivial::severity] << " | "
           << rec[boost::log::expressions::smessage];
}

void setFileSink()
{
    std::filesystem::path file_path(base::config::LOG_FOLDER);
    file_path /= std::filesystem::path(base::config::LOG_FILE_FORMAT);

    using TextFileSink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>;
    auto sink = boost::make_shared<TextFileSink>(boost::log::keywords::file_name = file_path,
                                                 boost::log::keywords::max_size = base::config::LOG_FILE_MAX_SIZE,
                                                 boost::log::keywords::max_files = base::config::LOG_MAX_FILE_COUNT);

    sink->locked_backend()->auto_flush(true);

    sink->set_formatter(&formatter);

    boost::log::core::get()->add_sink(sink);
}

void setStdoutSink()
{
    using TextOstreamSink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;
    auto sink = boost::make_shared<TextOstreamSink>();

    boost::shared_ptr<std::ostream> stream(&std::clog, boost::null_deleter());
    sink->locked_backend()->add_stream(stream);
    sink->locked_backend()->auto_flush(true);

    sink->set_formatter(&formatter);
    sink->set_filter(logging::trivial::severity >= logging::trivial::debug);

    boost::log::core::get()->add_sink(sink);
}

void disableLogger()
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::fatal);
}

} // namespace


namespace base
{

void initLog(size_t mode)
{
    clearLoggerSettings();

    if (!mode) {
        disableLogger();
        return;
    }

    if (mode & base::Sink::STDOUT) {
        setStdoutSink();
    }
    if (mode & base::Sink::FILE) {
        setFileSink();
    }

    boost::log::add_common_attributes();
}

void dumpDebuggingInfo()
{
    LOG_DEBUG << boost::stacktrace::stacktrace();
}

void flushLog()
{
    boost::log::core::get()->flush();
}

} // namespace base
