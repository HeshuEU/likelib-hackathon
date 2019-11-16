#pragma once

#include "base/bytes.hpp"
#include "base/stringifiable_enum_class.hpp"

namespace net
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    PacketType, unsigned char, (HANDSHAKE)(PING)(PONG)(DISCOVER)(DATA)(DISCONNECT));

class Packet
{
  public:
    //===================
    Packet(PacketType type);
    //===================

    PacketType getType() const;
    void setType(PacketType type);

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
};

} // namespace net
