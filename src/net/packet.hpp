#pragma once

#include "base/bytes.hpp"

namespace net
{

class Packet
{
  public:
    //===================

    enum class Type : unsigned char
    {
        HANDSHAKE = 0,
        PING = 1,
        PONG = 2,
        DISCOVER = 3,
        DATA = 4,
        DISCONNECT = 5
    };

    //===================
    Packet(Type type);

    //===================

    Type getType();
    void setType(Type type);

    //===================
    base::Bytes serialize() const;
    static Packet deserialize(const base::Bytes& raw);
    //===================

    bool operator==(const Packet& another) const noexcept;
    bool operator!=(const Packet& another) const noexcept;

    //===================

  private:
    Packet() = default;

    Type _type;
};

} // namespace net
