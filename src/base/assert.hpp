#pragma once

#include "base/config.hpp"

#ifdef CONFIG_IS_DEBUG

#include "base/log.hpp"

#include <boost/stacktrace.hpp>

#include <exception>

#define CHECK(condition) \
    if(!(condition)) { \
        LOG_WARNING << "Assertion failed: " << #condition << '\n' << boost::stacktrace::stacktrace(); \
        std::exit(base::config::EXIT_ASSERT_FAILED); \
    }

#define CHECK_SOFT(condition) \
    if(!(condition)) { \
        LOG_WARNING << "Soft check failed: " << #condition << '\n' << boost::stacktrace::stacktrace(); \
    }

#else
#define CHECK(condition, message) \
    do { \
    } while(0)

#define CHECK_SOFT(condition, message) \
    do { \
    } while(0)
#endif
