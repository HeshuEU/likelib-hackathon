#include "protocol.hpp"

#include "base/log.hpp"

namespace bc
{

Message Message::blockBroadcast(const bc::Block& block)
{
    return {PacketType::BROADCAST_BLOCK, base::toBytes(block)};
}


Message Message::transactionBroadcast(const bc::Transaction& tx)
{
    return {PacketType::BROADCAST_BLOCK, base::toBytes(tx)};
}


Message Message::getBlock(const base::Sha256& hash)
{
    return {PacketType::GET_BLOCK, hash.getBytes()};
}


Message::Message(PacketType type, const base::Bytes& bytes) : _type{type}, _raw_data{bytes}
{}


Message::Message(PacketType type, base::Bytes&& bytes) : _type{type}, _raw_data(std::move(bytes))
{}


PacketType Message::getType() const noexcept
{
    return _type;
}


const base::Bytes& Message::getBytes() const& noexcept
{
    return _raw_data;
}


base::Bytes&& Message::getBytes() && noexcept
{
    return std::move(_raw_data);
}


Message Message::deserialize(const base::Bytes& bytes)
{
    base::SerializationIArchive ia(bytes);
    return deserialize(ia);
}


Message Message::deserialize(base::SerializationIArchive& ia)
{
    PacketType t;
    base::Bytes b;
    ia >> t >> b;
    return Message(t, std::move(b));
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Message& v)
{
    return oa << v.getType() << v.getBytes();
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Message& v)
{
    v = Message::deserialize(ia);
    return ia;
}


ProtocolEngine::ProtocolEngine(Blockchain& blockchain) : _blockchain{blockchain}
{}


void ProtocolEngine::handle(net::Connection& connection, base::Bytes&& bytes)
{
    auto packet = Message::deserialize(bytes);
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