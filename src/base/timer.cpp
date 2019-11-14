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
    return (now - _start_time).count();
}

unsigned long long Timer::elapsedSeconds() const
{
    return (now - _start_time).count();
}

} // namespace base