#include "base/log.hpp"

#include <exception>

int main()
{
    try {
        base::initLog(base::LogLevel::ALL, base::Sink::STDOUT | base::Sink::FILE);

        LOG_INFO << "Application startup";
    }
    catch(const std::exception& error) {
        LOG_ERROR << "exception caught in main: " << error.what();
    }
    catch(...) {
        LOG_ERROR << "unknown exception caught";
    }
}