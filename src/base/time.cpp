#include "time.hpp"

namespace base
{

Time::Time()
{}

Time::Time(std::chrono::time_point<std::chrono::system_clock> time_point) : _time(time_point)
{}

long Time::millisecondsInEpoch() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(_time.time_since_epoch()).count();
}


long Time::secondsInEpoch() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(_time.time_since_epoch()).count();
}

Time Time::now()
{
    return Time(std::chrono::system_clock::now());
}

Time Time::fromMilliseconds(long milliseconds_from_epoch)
{
    std::chrono::duration<long, std::ratio<1, 1000>> duration_from_epoch(milliseconds_from_epoch);
    return Time(std::chrono::time_point<std::chrono::system_clock>(duration_from_epoch));
}

Time Time::fromSeconds(long seconds_from_epoch)
{
    std::chrono::duration<long, std::ratio<1, 1>> duration_from_epoch(seconds_from_epoch);
    return Time(std::chrono::time_point<std::chrono::system_clock>(duration_from_epoch));
}

} // namespace base