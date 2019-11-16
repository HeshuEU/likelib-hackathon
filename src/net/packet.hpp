#pragma once

#include "base/bytes.hpp"
#include "base/stringifiable_enum_class.hpp"

#include <string>
#include <vector>

namespace net
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    PacketType, unsigned char, (HANDSHAKE)(PING)(PONG)(DISCOVERY)(DATA)(DISCONNECT));

class Packet
{
  public:
    //===================
    Packet(PacketType type);
    //===================
    PacketType getType() const;
    void setType(PacketType type);

    const std::vector<std::string> getKnownEndpoints() const;
    void setKnownEndpoints(std::vector<std::string>&& endpoints);
    //===================
    base::Bytes serialize() const;
    static Packet deserialize(const base::Bytes& raw);
    //===================

    bool operator==(const Packet& another) const noexcept;
    bool operator!=(const Packet& another) const noexcept;

    //===================

  private:
    Packet() = default;

    PacketType _type;
    std::vector<std::string> _known_endpoints;
};

} // namespace net
