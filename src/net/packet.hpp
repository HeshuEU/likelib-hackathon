#pragma once

#include "base/bytes.hpp"
#include "base/stringifiable_enum_class.hpp"
#include "net/endpoint.hpp"

#include <string>
#include <vector>

namespace net
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(PacketType, unsigned char,
    (HANDSHAKE)(PING)(PONG)(DISCOVERY_REQ)(DISCOVERY_RES)(DATA)(BLOCK)(TRANSACTION)(DISCONNECT));

class Packet
{
  public:
    //===================
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
    base::Bytes serialize() const;
    static Packet deserialize(const base::Bytes& raw);
    //===================

    bool operator==(const Packet& another) const noexcept;
    bool operator!=(const Packet& another) const noexcept;

    //===================

  private:
    Packet() = default;

    PacketType _type{PacketType::DISCONNECT};
    std::vector<std::string> _known_endpoints;
    unsigned short _server_public_port{0};
};

} // namespace net
