#include "protocol.hpp"

#include "base/log.hpp"
#include "lk/core.hpp"

namespace lk
{

Network::Network(const base::PropertyTree& config, Core& core) : _config(config), _host{_config}, _handler{core, _host}
{}


void Network::broadcastBlock(const bc::Block& block)
{
    base::SerializationOArchive oa;
    oa << MessageType::BLOCK << block;
    _host.broadcast(std::move(oa).getBytes());
}


void Network::broadcastTransaction(const bc::Transaction& tx)
{
    base::SerializationOArchive oa;
    oa << MessageType::TRANSACTION << tx;
    _host.broadcast(std::move(oa).getBytes());
}


void Network::run()
{
    _host.run([this](net::Session& session, const base::Bytes& received_data) {
        _handler.handle(session, received_data);
    });
}


MessageHandler::MessageHandler(Core& core, net::Host& host) : _core{core}, _host{host}
{}


void MessageHandler::handle(net::Session& session, const base::Bytes& data)
{
    if(session.isClosed()) {
        return;
    }

    base::SerializationIArchive ia(data);
    MessageType type;
    ia >> type;
    switch(type) {
        case MessageType::PING: {
            break;
        }
        case MessageType::PONG: {
            break;
        }
        case MessageType::TRANSACTION: {
            bc::Transaction tx;
            ia >> tx;
            onTransaction(session, std::move(tx));
            break;
        }
        case MessageType::BLOCK: {
            auto block = bc::Block::deserialize(ia);
            onBlock(session, std::move(block));
            break;
        }
        case MessageType::GET_BLOCK: {
            base::Bytes block_hash;
            ia >> block_hash;
            onGetBlock(session, base::Sha256(block_hash));
            break;
        }
        default: {
            LOG_DEBUG << "Received an invalid block from peer #123";
            break;
        }
    }
}


void MessageHandler::onPing(net::Session& session)
{
    base::SerializationOArchive msg;
    msg << MessageType::PONG;
    session.send(std::move(msg).getBytes());
}


void MessageHandler::onPong(net::Session& session)
{}


void MessageHandler::onTransaction(net::Session& session, bc::Transaction&& tx)
{
    LOG_TRACE << "Session::onTransaction peer " << session.getId();
    _core.performTransaction(tx);
}


void MessageHandler::onBlock(net::Session& session, bc::Block&& block)
{
    LOG_TRACE << "Session::onBlock peer " << session.getId();
    _core.tryAddBlock(std::move(block));
}


void MessageHandler::onGetBlock(net::Session& session, base::Sha256&& block_hash)
{
    LOG_TRACE << "Session::onGetBlock peer " << session.getId();
    auto block = _core.findBlock(block_hash);
    if(block) {
        base::SerializationOArchive msg;
        msg << MessageType::BLOCK << *block;
        session.send(std::move(msg).getBytes());
    }
}

} // namespace lk