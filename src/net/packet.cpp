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


const base::Bytes& Packet::getData() const noexcept
{
    return _data;
}


void Packet::setData(const base::Bytes& data)
{
    _data = data;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Packet& p)
{
    return oa << static_cast<int>(p.getType()) << p.getKnownEndpoints() << p.getPublicServerPort() << p.getData();
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Packet& p)
{
    PacketType ptype;
    int x;
    ia >> x;
    ptype = static_cast<PacketType>(x);

    if(!enumToString(ptype)) { // enumToString returns nullptr if enum doesn't have given value
        RAISE_ERROR(net::Error, "cannot deserialize an invalid packet");
    }

    p.setType(ptype);

    std::vector<std::string> known_endpoints;
    unsigned short server_public_port;
    base::Bytes data;

    ia >> known_endpoints >> server_public_port >> data;
    p.setKnownEndpoints(std::move(known_endpoints));
    p.setPublicServerPort(server_public_port);
    p.setData(data);

    return ia;
}


} // namespace net