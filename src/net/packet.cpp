#include "packet.hpp"

#include "net/error.hpp"

// portable archives
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <sstream>

namespace net
{

Packet::Packet(PacketType type) : _type{type}
{}


PacketType Packet::getType() const
{
    return _type;
}


void Packet::setType(PacketType type)
{
    _type = type;
}


base::Bytes Packet::serialize() const
{
    std::ostringstream oss;
    boost::archive::text_oarchive to(oss);
    to << _type;

    return base::Bytes(oss.str());
}


Packet Packet::deserialize(const base::Bytes& raw)
{
    std::istringstream iss(raw.toString());
    boost::archive::text_iarchive ti(iss);

    Packet ret;
    ti >> ret._type;

    if(!enumToString(ret._type)) { // enumToString returns nullptr if enum doesn't have given value
        RAISE_ERROR(net::Error, "received an invalid packet");
    }

    return ret;
}


bool Packet::operator==(const Packet& another) const noexcept
{
    return _type == another._type;
}


bool Packet::operator!=(const Packet& another) const noexcept
{
    return !(*this == another);
}

} // namespace net