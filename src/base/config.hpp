#pragma once

#include <cstddef>

namespace base::config
{

// IS_DEBUG definition
#if defined(NDEBUG)
constexpr bool IS_DEBUG = false;
#else
constexpr bool IS_DEBUG = true;
#endif
//------------------------

// OS_NAME definition
constexpr const char* const OS_NAME =
#if defined(_WIN32) || defined(_WIN64)
    "Windows"
#elif defined(__APPLE__) || defined(__MACH__)
    "Mac OSX"
#elif defined(__linux__)
    "Linux"
#elif defined(__FreeBSD__)
    "FreeBSD"
#elif defined(__unix) || defined(__unix__)
    "Unix"
#else
    static_assert(false, "cannot determine OS")
#endif
    ;
//------------------------

// logging configuration
static constexpr const char* LOG_FILE_FORMAT = "app_%m-%d-%Y_%H:%M.log";
static constexpr const char* LOG_FOLDER = "logs";
static constexpr const int LOG_FILE_MAX_SIZE = 6 * 1024 * 1024;
static constexpr const int LOG_FILE_MIN_SPACE = 100 * 1024 * 1024;
static constexpr const int LOG_MAX_FILE_COUNT = 512;
//------------------------

} // namespace base::config