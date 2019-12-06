#pragma once

#include "base/bytes.hpp"
#include "base/stringifiable_enum_class.hpp"

#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "bc/transaction.hpp"

#include "net/connection.hpp"

namespace bc
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    PacketType, unsigned char, (BROADCAST_TRANSACTION)(BROADCAST_BLOCK)(GET_BLOCK)(RES_GET_BLOCK))

class Packet
{
  public:
    //===================
    [[nodiscard]] static Packet blockBroadcast(const bc::Block& block);
    [[nodiscard]] static Packet transactionBroadcast(const bc::Transaction& tx);
    [[nodiscard]] static Packet getBlock(const base::Sha256& hash);
    //===================
    [[nodiscard]] static Packet deserialize(const base::Bytes& bytes);
    [[nodiscard]] static Packet deserialize(base::SerializationIArchive& bytes);
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
};

base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Packet& v);
base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Packet& v);


class ProtocolEngine
{
public:
    //================
    ProtocolEngine(Blockchain& blockchain);
    //================
    void handle(net::Connection& connection, base::Bytes&& bytes);
    //================
private:
    //================
    Blockchain& _blockchain;
    //================
    void onGetBlock(net::Connection& connection, const base::Sha256& block_hash);
    void onResGetBlock(net::Connection& connection);
    //================
};


} // namespace bc