#include "time.hpp"

namespace base
{

std::uint_least32_t Time::getSecondsSinceEpoch() const
{
    return _seconds_since_epoch;
}


std::chrono::time_point<std::chrono::system_clock> Time::toTimePoint() const
{
    std::chrono::duration<std::uint32_t, std::ratio<1, 1>> duration_from_epoch(_seconds_since_epoch);
    return std::chrono::time_point<std::chrono::system_clock>(duration_from_epoch);
}


bool Time::operator==(const Time& other) const
{
    return _seconds_since_epoch == other._seconds_since_epoch;
}


bool Time::operator!=(const Time& other) const
{
    return !(*this == other);
}


Time Time::now()
{
    Time ret{ std::chrono::system_clock::now() };
    return ret;
}


Time::Time(std::uint_least32_t seconds_from_epoch)
  : _seconds_since_epoch{ seconds_from_epoch }
{}


Time::Time(std::chrono::time_point<std::chrono::system_clock> time_point)
  : _seconds_since_epoch{ static_cast<std::uint_least32_t>(
      std::chrono::duration_cast<std::chrono::seconds>(time_point.time_since_epoch()).count()) }
{}


void Time::serialize(SerializationOArchive& oa) const
{
    oa.serialize(static_cast<SerializationType>(_seconds_since_epoch));
}


Time Time::deserialize(SerializationIArchive& ia)
{
    auto timestamp = ia.deserialize<SerializationType>();
    return Time{ timestamp };
}


std::ostream& operator<<(std::ostream& os, const Time& time)
{
    return os << time.getSecondsSinceEpoch();
}

} // namespace base
