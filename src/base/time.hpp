#pragma once

#include "base/serialization.hpp"

#include <chrono>

namespace base
{

class Time
{
    //=======================
    friend ::base::SerializationIArchive& operator>>(::base::SerializationIArchive& ia, Time& tx);
    friend ::base::SerializationOArchive& operator<<(::base::SerializationOArchive& oa, const Time& tx);
    //==========================
  public:
    /**
     * \brief default constructor which have an epoch january 1, 1970.
     */
    Time() = default;
    Time(const Time&) = default;
    Time(Time&&) = default;
    //====================
    Time& operator=(const Time&) = default;
    Time& operator=(Time&&) = default;
    //====================
    ~Time() = default;
    //====================
    std::uint_least32_t getSecondsInEpoch() const;
    std::chrono::time_point<std::chrono::system_clock> toTimePoint() const;
    //====================
    bool operator==(const Time& other) const;
    bool operator!=(const Time& other) const;
    //====================
    static Time now();
    static Time fromSeconds(std::uint_least32_t seconds_from_epoch);
    static Time fromTimePoint(std::chrono::time_point<std::chrono::system_clock> time_point);
    //====================
  private:
    std::chrono::time_point<std::chrono::system_clock> _time;
    std::uint_least32_t _seconds_from_epoch_start{0};
    //=====================
};

::base::SerializationIArchive& operator>>(::base::SerializationIArchive& ia, Time& tx);
::base::SerializationOArchive& operator<<(::base::SerializationOArchive& oa, const Time& tx);

} // namespace base
