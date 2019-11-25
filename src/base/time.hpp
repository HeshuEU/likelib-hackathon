#pragma once

#include <chrono>
#include <string>

namespace base
{

class Time
{
  public:
    Time();

    Time(std::chrono::time_point<std::chrono::system_clock> time_point);

    long millisecondsInEpoch() const;

    long secondsInEpoch() const;

    static Time now();

    static Time fromMilliseconds(long milliseconds_from_epoch);

    static Time fromSeconds(long seconds_from_epoch);

  private:
    std::chrono::time_point<std::chrono::system_clock> _time;

};

} // namespace base