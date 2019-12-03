#pragma once

#include "base/bytes.hpp"
#include "base/serialization.hpp"
#include "base/stringifiable_enum_class.hpp"
#include "net/endpoint.hpp"

#include <string>
#include <vector>

namespace net
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(PacketType, unsigned char,
    (HANDSHAKE)(PING)(PONG)(DISCOVERY_REQ)(DISCOVERY_RES)(DATA)(BLOCK)(TRANSACTION)(DISCONNECT))

class Packet
{
  public:
    //===================
    Packet() = default;
    Packet(PacketType type);
    //===================
    PacketType getType() const;
    void setType(PacketType type);
    //===================
    const std::vector<std::string> getKnownEndpoints() const;
    void setKnownEndpoints(std::vector<std::string>&& endpoints);
    //===================
    unsigned short getPublicServerPort() const noexcept;
    void setPublicServerPort(unsigned short endpoint);
    //===================
    const base::Bytes& getData() const noexcept;
    void setData(const base::Bytes& data);
    //===================
    bool operator==(const Packet& another) const noexcept;
    bool operator!=(const Packet& another) const noexcept;
    //===================

  private:
    PacketType _type{PacketType::DISCONNECT};
    std::vector<std::string> _known_endpoints;
    unsigned short _server_public_port{0};
    base::Bytes _data;
};

base::SerializationOArchive& operator<<(base::SerializationOArchive& ia, const Packet& v);
base::SerializationIArchive& operator>>(base::SerializationIArchive& oa, Packet& v);

} // namespace net
