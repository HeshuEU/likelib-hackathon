#pragma once

#include "base/bytes.hpp"
#include "base/serialization.hpp"
#include "base/stringifiable_enum_class.hpp"
#include "net/endpoint.hpp"

#include <string>
#include <vector>

namespace net
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    PacketType, unsigned char, (HANDSHAKE)(PING)(PONG)(DISCOVERY_REQ)(DISCOVERY_RES)(DATA)(DISCONNECT))

class Packet
{
  public:
    //===================
    Packet() = default;
    Packet(PacketType type);
    //===================
    [[nodiscard]] PacketType getType() const;
    void setType(PacketType type);
    //===================
    [[nodiscard]] const base::Bytes& getData() const& noexcept;
    [[nodiscard]] base::Bytes&& getData() && noexcept;
    void setData(const base::Bytes& data);
    //===================
    bool operator==(const Packet& another) const noexcept;
    bool operator!=(const Packet& another) const noexcept;
    //===================

  private:
    PacketType _type{PacketType::DISCONNECT};
    base::Bytes _data;
};

base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Packet& v);
base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Packet& v);

} // namespace net
