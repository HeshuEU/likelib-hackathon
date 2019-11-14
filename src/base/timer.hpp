#pragma once

#include <chrono>

namespace base
{


class Timer
{
  public:
    //=================
    Timer() = default;
    ~Timer() = default;
    //=================
    void start();

    unsigned long long elapsedMillis() const;
    unsigned long long elapsedSeconds() const;
    //=================
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock>_start_time;
};

} // namespace base