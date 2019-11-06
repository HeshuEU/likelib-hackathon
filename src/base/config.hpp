#pragma once

#include <cstddef>

namespace base::config
{

// constexpr IS_DEBUG definition
constexpr bool IS_DEBUG =
#if defined(NDEBUG)
        false
#else
        true
#endif
;

// constexpr IS_DEBUG definition && macro CONFIG_IS_DEBUG or CONFIG_IS_RELEASE definition
#if defined(NDEBUG)
#define CONFIG_IS_RELEASE
#else
#define CONFIG_IS_DEBUG
#endif
//------------------------

// macro CONFIG_OS_FAMILY_NAME definition
#if defined(_WIN32) || defined(_WIN64)
#define CONFIG_OS_FAMILY_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#define CONFIG_OS_FAMILY_MAC_OSX
#elif defined(__linux__)
#define CONFIG_OS_FAMILY_UNIX
#elif defined(__FreeBSD__)
#define CONFIG_OS_FAMILY_UNIX
#elif defined(__unix) || defined(__unix__)
#define CONFIG_OS_FAMILY_UNIX
#else
    static_assert(false, "cannot determine OS")
#endif
    ;
//------------------------

// constexpr OS_NAME definition
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

// exit codes
static constexpr int EXIT_OK = 0;
static constexpr int EXIT_FAIL = 1;
static constexpr int EXIT_ASSERT_FAILED = 2;

} // namespace base::config