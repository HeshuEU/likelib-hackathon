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
    Time(const Time&) = default;
    Time(Time&&) = default;
    //====================
    Time& operator=(const Time&) = default;
    Time& operator=(Time&&) = default;
    //====================
    ~Time() = default;
    //====================
    [[nodiscard]] std::uint_least32_t getSecondsSinceEpochBeginning() const;
    [[nodiscard]] std::chrono::time_point<std::chrono::system_clock> toTimePoint() const;
    //====================
    bool operator==(const Time& other) const;
    bool operator!=(const Time& other) const;
    //====================
    static Time now();
    static Time fromSecondsSinceEpochBeginning(std::uint_least32_t seconds_from_epoch);
    static Time fromTimePoint(std::chrono::time_point<std::chrono::system_clock> time_point);
    //=====================
    SerializationOArchive& serialize(SerializationOArchive& oa) const;
    static Time deserialize(SerializationIArchive& ia);
    //=====================
  private:
    //=====================
    std::uint_least32_t _seconds_since_epoch_beginning{0};
    //=====================
};

std::ostream& operator<<(std::ostream& os, const Time& time);

} // namespace base
