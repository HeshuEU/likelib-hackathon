#pragma once

#include "base/serialization.hpp"

#include <chrono>

namespace base
{

class Time
{
  public:
    Time() = default;
    Time(std::chrono::time_point<std::chrono::system_clock> time_point);
    Time(const Time&) = default;
    Time(Time&&) = default;
    //====================
    Time& operator=(const Time&) = default;
    Time& operator=(Time&&) = default;
    //====================
    ~Time() = default;
    //====================
    std::uint_least32_t millisecondsInEpoch() const;
    std::uint_least32_t secondsInEpoch() const;
    //====================
    bool operator==(const Time& other) const;
    bool operator!=(const Time& other) const;
    //====================
    static Time now();
    static Time fromMilliseconds(std::uint_least32_t milliseconds_from_epoch);
    static Time fromSeconds(std::uint_least32_t seconds_from_epoch);
    //====================
  private:
    std::chrono::time_point<std::chrono::system_clock> _time;
    //=====================
};

::base::SerializationIArchive& operator>>(::base::SerializationIArchive& ia, Time& tx);
::base::SerializationOArchive& operator<<(::base::SerializationOArchive& oa, const Time& tx);

} // namespace base
