#include <boost/test/unit_test.hpp>

#include "base/log.hpp"
#include "base/config.hpp"

#include <string>
#include <filesystem>
#include <fstream>

static constexpr const char *REQUIRED_LINE = "REQUIRED LINE";
static constexpr const char *EXTRA_LINE = "EXTRA LINE";
static constexpr size_t REQUIRED_LINE_NUMBER = 1;

std::filesystem::path freeFile() {
    std::filesystem::path file_path(base::config::LOG_FOLDER);
    file_path += std::filesystem::path::preferred_separator;

    { // copy from log.cpp
        std::time_t raw_time;
        std::tm *time_info;
        char buffer[80];
        std::time(&raw_time);
        time_info = std::localtime(&raw_time);
        std::strftime(buffer, sizeof(buffer), base::config::LOG_FILE_FORMAT, time_info);
        file_path += std::filesystem::path(buffer);
    }

    if (std::filesystem::exists(file_path)) {
        BOOST_ASSERT(std::filesystem::remove(file_path));
    }
    return file_path;
}


void initLog() {
    base::initLog(base::LogLevel::ERROR, base::Sink::FILE);
    LOG_TRACE << EXTRA_LINE;
    LOG_DEBUG << "test message for debug log fn";
    LOG_INFO << EXTRA_LINE;
    LOG_WARNING << "test message for warning log fn";
    LOG_ERROR << REQUIRED_LINE;
    LOG_FATAL << "test message for fatal log fn";
}

bool checkLogFile(std::filesystem::path file_path) {
    std::ifstream log_file_stream;

    log_file_stream.open(file_path.c_str());
    if (!log_file_stream) {
        return false;
    }

    std::string line;
    size_t required_line_counter = 0;
    while (std::getline(log_file_stream, line)) {
        if (line.find(EXTRA_LINE) != std::string::npos) {
            return false;
        }
        if (line.find(REQUIRED_LINE) != std::string::npos) {
            required_line_counter++;
        }
    }
    log_file_stream.close();

    if (required_line_counter != REQUIRED_LINE_NUMBER) {
        return false;
    }

    return true;
}

BOOST_AUTO_TEST_CASE(log_init_file) {
    // check file exists if exists delete
    auto file_path = freeFile();
    // init file and fill
    initLog();
    // check content
    if (std::filesystem::exists(file_path)) {
        BOOST_ASSERT(checkLogFile(file_path));
    } else {
        //try any time
        file_path = freeFile();
        initLog();
        BOOST_ASSERT (std::filesystem::exists(file_path));
        BOOST_ASSERT (checkLogFile(file_path));
    }
}
