#pragma once

#include <filesystem>
#include <optional>
#include <string>


template<typename... Args>
void printError(const Args&... args);


template<typename... Args>
void printUnexpectedError(const Args&... args);


#include "utility.tpp"
