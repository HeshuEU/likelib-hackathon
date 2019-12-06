#include "protocol.hpp"

#include "base/log.hpp"

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


Packet Packet::deserialize(const base::Bytes& bytes)
{
    base::SerializationIArchive ia(bytes);
    return deserialize(ia);
}


Packet Packet::deserialize(base::SerializationIArchive& ia)
{
    PacketType t;
    base::Bytes b;
    ia >> t >> b;
    return Packet(t, std::move(b));
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Packet& v)
{
    return oa << v.getType() << v.getBytes();
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Packet& v)
{
    v = Packet::deserialize(ia);
    return ia;
}


ProtocolEngine::ProtocolEngine(Blockchain& blockchain) : _blockchain{blockchain}
{}


void ProtocolEngine::handle(net::Connection& connection, base::Bytes&& bytes)
{
    auto packet = Packet::deserialize(bytes);
    switch(packet.getType()) {
        case PacketType::GET_BLOCK: {
            base::Sha256 hash(std::move(bytes));
            onGetBlock(connection, hash);
            break;
        }
        case PacketType::RES_GET_BLOCK: {
            base::SerializationIArchive ia(packet.getBytes());
            bool is_block_found;
            ia >> is_block_found;
            if(!is_block_found) {
                // onResGetBlock();
            }
            else {
                Block b;
                ia >> b;
                // onResGetBlock(std::move(b));
            }
            break;
        }
        case PacketType::BROADCAST_BLOCK: {
            break;
        }
        case PacketType::BROADCAST_TRANSACTION: {
            break;
        }
        default: {
            LOG_WARNING << "Received an invalid block";
        }
    }
}


void ProtocolEngine::onGetBlock(net::Connection& connection, const base::Sha256& block_hash)
{
    if(auto block_opt = _blockchain.findBlock(block_hash)) {
        Block& block = *block_opt;
    }
    else {
    }
}

} // namespace bc