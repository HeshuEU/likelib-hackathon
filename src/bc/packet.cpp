#include "packet.hpp"

namespace bc
{

Packet Packet::blockBroadcast(const bc::Block& block)
{
    return {PacketType::BROADCAST_BLOCK, base::toBytes(block)};
}


Packet Packet::transactionBroadcast(const bc::Transaction& tx)
{
    return {PacketType::BROADCAST_BLOCK, base::toBytes(tx)};
}


Packet Packet::getBlock(const base::Sha256& hash)
{
    return {PacketType::GET_BLOCK, hash.getBytes()};
}


Packet::Packet(PacketType type, const base::Bytes& bytes) : _type{type}, _raw_data{bytes}
{}


Packet::Packet(PacketType type, base::Bytes&& bytes) : _type{type}, _raw_data(std::move(bytes))
{}


PacketType Packet::getType() const noexcept
{
    return _type;
}


const base::Bytes& Packet::getBytes() const& noexcept
{
    return _raw_data;
}


base::Bytes&& Packet::getBytes() && noexcept
{
    return std::move(_raw_data);
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Packet& v)
{
    oa << v.getType() << v.getBytes();
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Packet& v)
{
    PacketType t;
    base::Bytes b;
    ia >> t >> b;
    v = Packet(t, std::move(b));
    return ia;
}

} // namespace bc