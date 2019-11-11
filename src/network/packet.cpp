#include "packet.hpp"

// portable archives
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/binary_object.hpp>

#include <sstream>

namespace network
{

Packet::Packet(std::size_t version) : _version{version}
{}


std::size_t Packet::getVersion()
{
    return _version;
}


void Packet::setVersion(std::size_t version)
{
    _version = version;
}


base::Bytes Packet::serialize() const
{
    std::ostringstream oss;
    boost::archive::text_oarchive to(oss);
    to << _version;

    return base::Bytes(oss.str());
}


Packet Packet::deserialize(const base::Bytes& raw)
{
    std::istringstream iss(raw.toString());
    boost::archive::text_iarchive ti(iss);

    Packet ret;
    ti >> ret._version;

    return ret;
}


bool Packet::operator==(const Packet& another) const noexcept
{
    return _version == another._version;
}


bool Packet::operator!=(const Packet& another) const noexcept
{
    return !(*this == another);
}

} // namespace network