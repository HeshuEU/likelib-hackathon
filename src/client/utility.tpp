#pragma once

#include "utility.hpp"

#include <iostream>
#include <utility>

namespace impl {

template<typename... Args>
void stdErrPrintLine(const Args&... args) {
    ((std::cerr << args << ' '), ...);
    std::cerr << '\n';
}

template<typename... Args>
void stdOutPrintLine(const Args&... args) {
    ((std::cerr << args << ' '), ...);
    std::cerr << '\n';
}

}


template<typename... Args>
void printError(const Args&... args) {
    stdErrPrintLine("Error:", std::forward<Args>(args)...);
}

template<typename... Args>
void printUnexpectedError(const Args&... args) {
    stdErrPrintLine("Unexpected error:", std::forward<Args>(args)...);
}
