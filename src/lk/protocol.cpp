#include "protocol.hpp"

#include "base/log.hpp"

namespace lk
{

MessageHandlerRouter::MessageHandlerRouter(bc::Blockchain& blockchain, net::Host& host, net::Peer& peer)
    : _blockchain{blockchain}, _host{host}, _peer{peer}
{}


void MessageHandlerRouter::handle(const base::Bytes& data)
{
    base::SerializationIArchive ia(data);
    MessageType type;
    ia >> type;
    switch(type) {
        case MessageType::TRANSACTION: {
            bc::Transaction tx;
            ia >> tx;
            onTransaction(std::move(tx));
            break;
        }
        case MessageType::BLOCK: {
            bc::Block block;
            ia >> block;
            onBlock(std::move(block));
            break;
        }
        case MessageType::GET_BLOCK: {
            base::Bytes block_hash;
            ia >> block_hash;
            onGetBlock(base::Sha256(block_hash));
            break;
        }
        default: {
            LOG_DEBUG << "Received an invalid block from peer " << _peer.getId();
            break;
        }
    }
}


void MessageHandlerRouter::onTransaction(bc::Transaction&& tx)
{
    if(!!_blockchain.findTransaction(base::toBytes(tx))) {
        return;
    }
    base::SerializationOArchive msg;
    msg << MessageType::TRANSACTION << tx;
    _host.broadcast(std::move(msg).getBytes());
}


void MessageHandlerRouter::onBlock(bc::Block&& block)
{
    if(_blockchain.tryAddBlock(block)) {
        base::SerializationOArchive msg;
        msg << MessageType::BLOCK << block;
        _host.broadcast(std::move(msg).getBytes());
    }
}


void MessageHandlerRouter::onGetBlock(base::Sha256&& block_hash)
{
    auto block = _blockchain.findBlock(block_hash);
    if(block) {
        base::SerializationOArchive msg;
        msg << MessageType::BLOCK << *block;
        _peer.send(std::move(msg).getBytes());
    }
}


ProtocolEngine::ProtocolEngine(const base::PropertyTree& config, bc::Blockchain& blockchain)
    : _config{config}, _host{_config}, _blockchain{blockchain}
{}


void ProtocolEngine::run()
{
    acceptLoop();
    _host.run();
    connectToPeersFromConfig();
}


void ProtocolEngine::acceptLoop()
{
    LOG_INFO << "listening for new connections";
    _host.accept([this](std::shared_ptr<net::Peer> peer) {
        LOG_INFO << "peer accepted";
        onAccept(*peer);
        acceptLoop();
    });
}


void ProtocolEngine::connectToPeersFromConfig()
{
    if(_config.hasKey("nodes")) {
        auto nodes = _config.getVector<std::string>("nodes");
        for(const auto& node: nodes) {
            _host.connect(net::Endpoint{node}, [this](std::shared_ptr<net::Peer> node) {
                onConnect(*node);
            });
        }
    }
}


void ProtocolEngine::onAccept(net::Peer& peer)
{
    _routers.emplace(peer.getId(), MessageHandlerRouter(_blockchain, _host, peer));
    peer.receive([this, &peer](const base::Bytes& received_data) {
        onReceive(peer, received_data);
    });
}


void ProtocolEngine::onConnect(net::Peer& peer)
{
    _routers.emplace(peer.getId(), MessageHandlerRouter(_blockchain, _host, peer));
}


void ProtocolEngine::onReceive(net::Peer& peer, const base::Bytes& received_data)
{
    ASSERT(_routers.find(peer.getId()) != _routers.end());
    LOG_DEBUG << "Received [" << received_data.size() << "] bytes";
    auto& [id, router] = *_routers.find(peer.getId());
    router.handle(received_data);
}


void ProtocolEngine::broadcastBlock(const bc::Block& block)
{
    base::SerializationOArchive oa;
    oa << MessageType::BLOCK << block;
    _host.broadcast(std::move(oa).getBytes());
}


void ProtocolEngine::broadcastTransaction(const bc::Transaction & tx)
{
    base::SerializationOArchive oa;
    oa << MessageType::TRANSACTION << tx;
    _host.broadcast(std::move(oa).getBytes());
}

} // namespace lk