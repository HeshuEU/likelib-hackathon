#pragma once

#include "base/serialization.hpp"

#include <chrono>

namespace base
{

/// Time util class with second accuracy in measurement
class Time
{
  public:
    /**
     * \brief default constructor which have an epoch january 1, 1970.
     */
    Time() = default;
    explicit Time(std::uint_least32_t seconds_from_epoch);
    explicit Time(std::chrono::time_point<std::chrono::system_clock> time_point);
    Time(const Time&) = default;
    Time(Time&&) = default;
    //====================
    Time& operator=(const Time&) = default;
    Time& operator=(Time&&) = default;
    //====================
    ~Time() = default;
    //====================
    std::uint_least32_t getSeconds() const;
    std::chrono::time_point<std::chrono::system_clock> toTimePoint() const;
    //====================
    bool operator==(const Time& other) const;
    bool operator!=(const Time& other) const;
    bool operator<(const Time& other) const;
    bool operator>(const Time& other) const;
    bool operator<=(const Time& other) const;
    bool operator>=(const Time& other) const;
    //====================
    Time operator-(const Time& other) const;
    //====================
    static Time now();
    //=====================
    void serialize(SerializationOArchive& oa) const;
    static Time deserialize(SerializationIArchive& ia);
    //=====================
  private:
    //=====================
    using SerializationType = std::uint32_t;
    std::uint_least32_t _seconds_since_epoch{ 0 };
    //=====================
};

std::ostream& operator<<(std::ostream& os, const Time& time);

class Timer
{
  public:
    //=================
    Timer() = default;
    ~Timer() = default;
    //=================
    void start();

    unsigned long long elapsedMillis() const;
    double elapsedSeconds() const;
    //=================
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
};

} // namespace base
