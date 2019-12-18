#include "packet.hpp"

#include "base/serialization.hpp"
#include "net/error.hpp"

#include <sstream>
#include <vector>

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


const base::Bytes& Packet::getData() const& noexcept
{
    return _data;
}


base::Bytes&& Packet::getData() && noexcept
{
    return std::move(_data);
}


void Packet::setData(const base::Bytes& data)
{
    _data = data;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Packet& p)
{
    return oa << static_cast<int>(p.getType()) << p.getData();
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Packet& p)
{
    PacketType type;
    int x;
    ia >> x;
    type = static_cast<PacketType>(x);

    if(!enumToString(type)) { // enumToString returns nullptr if enum doesn't have given value
        RAISE_ERROR(net::Error, "cannot deserialize an invalid packet");
    }

    p.setType(type);

    base::Bytes data;
    ia >> data;
    p.setData(data);

    return ia;
}


} // namespace net