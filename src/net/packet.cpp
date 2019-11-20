#include "packet.hpp"

#include "net/error.hpp"

// portable archives
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

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
    boost::archive::text_oarchive to(oss, boost::archive::no_header);
    to << _type << _known_endpoints << _server_public_port;

    return base::Bytes(oss.str());
}


Packet Packet::deserialize(const base::Bytes& raw)
{
    std::istringstream iss(raw.toString());
    boost::archive::text_iarchive ti(iss, boost::archive::no_header);

    Packet ret;
    ti >> ret._type;

    if(!enumToString(ret._type)) { // enumToString returns nullptr if enum doesn't have given value
        RAISE_ERROR(net::Error, "received an invalid packet");
    }

    ti >> ret._known_endpoints >> ret._server_public_port;

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


const std::vector<std::string> Packet::getKnownEndpoints() const
{
    return _known_endpoints;
}


void Packet::setKnownEndpoints(std::vector<std::string>&& endpoints)
{
    _known_endpoints = std::move(endpoints);
}


unsigned short Packet::getPublicServerPort() const noexcept
{
    return _server_public_port;
}


void Packet::setPublicServerPort(unsigned short endpoint)
{
    _server_public_port = endpoint;
}

} // namespace net