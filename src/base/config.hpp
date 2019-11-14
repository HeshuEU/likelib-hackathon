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
constexpr const char* LOG_FILE_FORMAT = "%m-%d-%Y_%H-%M.log";
constexpr const char* LOG_FOLDER = "logs";
constexpr const std::size_t LOG_FILE_MAX_SIZE = 5 * 1024 * 1024; // 5 mb max log file
constexpr const std::size_t LOG_MAX_FILE_COUNT = 512; // max log files
//------------------------

// exit codes
constexpr int EXIT_OK = 0;
constexpr int EXIT_FAIL = 1;
// to better see when ASSERTIONS failed during debug
constexpr int EXIT_ASSERT_FAILED = 2;
//------------------------

// network
constexpr std::size_t NETWORK_MESSAGE_BUFFER_SIZE = 16 * 1024; // 16KB
//------------------------

} // namespace base::config