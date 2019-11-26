#pragma once

#include <chrono>

namespace base
{

class Time
{
  public:
    Time();

    Time(std::chrono::time_point<std::chrono::system_clock> time_point);

    std::uint_least32_t millisecondsInEpoch() const;

    std::uint_least32_t secondsInEpoch() const;

    static Time now();

    static Time fromMilliseconds(std::uint_least32_t milliseconds_from_epoch);

    static Time fromSeconds(std::uint_least32_t seconds_from_epoch);

  private:
    std::chrono::time_point<std::chrono::system_clock> _time;
};

} // namespace base