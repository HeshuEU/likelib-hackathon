#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <thread>

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
static_assert(false, "cannot determine OS");
#endif

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
constexpr const char* LOG_FILE_FORMAT = "%m-%d-%Y_%H-%M-%S_%N.log";
constexpr const char* LOG_FOLDER = "logs";
constexpr const std::size_t LOG_FILE_MAX_SIZE = 5 * 1024 * 1024; // 5 mb max log file
constexpr const std::size_t LOG_MAX_FILE_COUNT = 512;            // max log files
//------------------------

// exit codes
constexpr int EXIT_OK = 0;
constexpr int EXIT_FAIL = 1;
// to better see when ASSERTIONS failed during debug
constexpr int EXIT_ASSERT_FAILED = 2;
//------------------------

// net
constexpr std::size_t NET_MESSAGE_BUFFER_SIZE = 16 * 1024; // 16KB
constexpr std::size_t NET_PING_FREQUENCY = 3600;           // seconds
constexpr std::size_t NET_CONNECT_TIMEOUT = 10;            // seconds
constexpr std::size_t NET_LOOKUP_ALPHA = 5;                // how many peers to return during lookup
constexpr std::size_t NET_REQUEST_TIMEOUT = 10; // how many seconds do we wait for a request, until we call it lost
//------------------------

// blockchain
constexpr std::size_t BC_MAX_TRANSACTIONS_IN_BLOCK = 100;
constexpr std::size_t BC_TARGET_BLOCKS_PER_MINUTE = 1;      // block every 60 / 12 == 5 seconds
constexpr std::size_t BC_DIFFICULTY_RECALCULATION_RATE = 2; // how many blocks must be added to recalculate difficulty
constexpr std::size_t BC_MAXIMAL_CHANGE_MULTIPLIER = 1'000'000'000; // times complexity could change at once
constexpr std::size_t BC_EMISSION_VALUE = 1000;
//------------------------

// websocket
constexpr const std::uint32_t RPC_PUBLIC_API_VERSION = 1;
constexpr std::size_t RPC_MESSAGE_BUFFER_SIZE = 16 * 1024; // 16KB
//--------------------

// database
constexpr std::size_t DATABASE_WRITE_BUFFER_SIZE = 50 * 1024 * 1024;     // 50MB write buffer
constexpr std::size_t DATABASE_DATA_BLOCK_SIZE = 10 * 1024;              // 10KB data-block size
constexpr std::size_t DATABASE_DATA_BLOCK_CACHE_SIZE = 50 * 1024 * 1024; // 50MB data-block cache size
constexpr bool DATABASE_COMPRESS_DATA = false;                           // no compress data
//--------------------

// keys paths
std::filesystem::path makePrivateKeyPath(const std::filesystem::path& path);

} // namespace base::config
