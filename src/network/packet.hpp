#pragma once

#include "base/bytes.hpp"

namespace network
{

class Packet
{
  public:
    //===================
    Packet(std::size_t version);

    //===================

    std::size_t getVersion();
    void setVersion(std::size_t version);

    //===================
    base::Bytes serialize() const;
    static Packet deserialize(const base::Bytes& raw);
    //===================

  private:
    Packet() = default;

    std::size_t _version;
};

} // namespace network
