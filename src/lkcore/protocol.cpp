#include "protocol.hpp"

#include "base/log.hpp"


namespace
{

template<typename H, typename... Args>
void runAllHandlers(std::vector<H>& handlers, Args&&... args)
{
    for(auto& handler: handlers) {
        handler(args...);
    }
}

} // namespace


namespace bc
{

Message Message::blockBroadcast(const bc::Block& block)
{
    return {MessageType::BLOCK, base::toBytes(block)};
}


Message Message::transactionBroadcast(const bc::Transaction& tx)
{
    return {MessageType::BLOCK, base::toBytes(tx)};
}


Message Message::getBlock(const base::Sha256& hash)
{
    return {MessageType::GET_BLOCK, hash.getBytes()};
}


Message::Message(MessageType type, const base::Bytes& bytes) : _type{type}, _raw_data{bytes}
{}


Message::Message(MessageType type, base::Bytes&& bytes) : _type{type}, _raw_data(std::move(bytes))
{}


MessageType Message::getType() const noexcept
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
    MessageType t;
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


void MessageHandlerManager::handle(net::Peer& peer, base::Bytes&& bytes)
{
    auto packet = Message::deserialize(bytes);
    switch(packet.getType()) {
        case MessageType::GET_BLOCK: {
            base::Sha256 hash(std::move(bytes));
            runAllHandlers(_on_get_block_handlers, hash);
            break;
        }
        case MessageType::BLOCK: {
            base::SerializationIArchive ia(packet.getBytes());
            Block b;
            ia >> b;
            runAllHandlers(_on_block_handlers, b);
            break;
        }
        case MessageType::TRANSACTION: {
            base::SerializationIArchive ia(packet.getBytes());
            Transaction tx;
            ia >> tx;
            runAllHandlers(_on_transaction_handlers, tx);
            break;
        }
        default: {
            LOG_WARNING << "Received an invalid block";
        }
    }
}

} // namespace bc