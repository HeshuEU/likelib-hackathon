#pragma once

#include "base/bytes.hpp"
#include "base/stringifiable_enum_class.hpp"

#include "bc/block.hpp"
#include "bc/transaction.hpp"

namespace bc
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    PacketType, unsigned char, (BROADCAST_TRANSACTION)(BROADCAST_BLOCK)(GET_BLOCK))


class Packet
{
  public:
    //===================
    [[nodiscard]] static Packet blockBroadcast(const bc::Block& block);
    [[nodiscard]] static Packet transactionBroadcast(const bc::Transaction& tx);
    [[nodiscard]] static Packet getBlock(const base::Sha256& hash);
    //===================
    [[nodiscard]] PacketType getType() const noexcept;
    [[nodiscard]] const base::Bytes& getBytes() const& noexcept;
    [[nodiscard]] base::Bytes&& getBytes() && noexcept;
    //===================
  private:
    //===================
    Packet(PacketType type, const base::Bytes& bytes);
    Packet(PacketType type, base::Bytes&& bytes);
    //===================
    PacketType _type;
    base::Bytes _raw_data;
    //===================
    friend base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Packet& v);
    //===================
};


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Packet& v);
base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Packet& v);


} // namespace bc