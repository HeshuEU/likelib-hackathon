#pragma once

namespace base::config
{

//common configuration values
constexpr bool IS_DEBUG = true;

constexpr const char* const OS_NAME = "Linux";

// logging configuration
static constexpr const char* LOG_FILE_FORMAT = "app_%m-%d-%Y_%H:%M.log";
static constexpr const char* LOG_FOLDER = "logs";
static constexpr const int LOG_FILE_MAX_SIZE = 6 * 1024 * 1024;
static constexpr const int LOG_FILE_MIN_SPACE = 100 * 1024 * 1024;
static constexpr const int MAX_LOG_FILE_COUNT = 512;

}