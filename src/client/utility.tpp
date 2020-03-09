#pragma once

#include "utility.hpp"

#include <iostream>

namespace impl
{

template<typename... Args>
void stdErrPrintLine(const Args&... args)
{
    ((std::cerr << args << ' '), ...);
    std::cerr << '\n';
}

template<typename... Args>
void stdOutPrintLine(const Args&... args)
{
    ((std::cerr << args << ' '), ...);
    std::cerr << '\n';
}

} // namespace impl


template<typename... Args>
void printError(const Args&... args)
{
    impl::stdErrPrintLine("Error:", args...);
}

template<typename... Args>
void printUnexpectedError(const Args&... args)
{
    impl::stdErrPrintLine("Unexpected error:", args...);
}
