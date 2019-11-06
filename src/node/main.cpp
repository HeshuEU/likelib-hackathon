#include "base/log.hpp"
#include "base/assert.hpp"

#include <exception>

int main(int argc, char** argv)
{
    try {
        base::initLog(base::LogLevel::ALL, base::Sink::STDOUT | base::Sink::FILE);

        CHECK(argc > 1);

        LOG_INFO << "Application startup";
    }
    catch(const std::exception& error) {
        LOG_ERROR << "exception caught in main: " << error.what();
    }
    catch(...) {
        LOG_ERROR << "unknown exception caught";
    }
}