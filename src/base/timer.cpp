#include "timer.hpp"

namespace base
{

void Timer::start()
{
    _start_time = std::chrono::high_resolution_clock::now();
}


unsigned long long Timer::elapsedMillis() const
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - _start_time).count();
}


double Timer::elapsedSeconds() const
{
    return elapsedMillis() / 1000.0;
}

} // namespace base