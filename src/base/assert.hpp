#pragma once

#include "base/config.hpp"
#include "base/log.hpp"

#include <exception>

#ifdef CONFIG_IS_DEBUG
#define CHECK(condition) \
    if(!(condition)) { \
        LOG_WARNING << "Assertion failed: " << #condition; \
        std::exit(base::config::EXIT_ASSERT_FAILED); \
    }
#else
#define CHECK(condition, message) \
    do { \
    } while(0)
#endif