#pragma once

#include "base/subprogram_router.hpp"

#include <filesystem>
#include <optional>
#include <string>


template<typename... Args>
void printError(const Args&... args);


template<typename... Args>
void printUnexpectedError(const Args&... args);


std::optional<std::filesystem::path> checkAndGetFilePath(const base::SubprogramRouter& router, const std::string_view& option_name);
std::optional<std::filesystem::path> checkAndGetDirectoryPath(const base::SubprogramRouter& router, const std::string_view& option_name);
