#include "protocol.hpp"

#include "base/log.hpp"
#include "lk/core.hpp"


namespace
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(MessageType, unsigned char,
    (NOT_AVAILABLE)(HANDSHAKE)(PING)(PONG)(TRANSACTION)(GET_BLOCK)(BLOCK)(BLOCK_NOT_FOUND)(GET_INFO)(INFO))

template<typename... Args>
base::Bytes createMessage(MessageType type, Args&&... args)
{
    base::SerializationOArchive ret;
    ret << type;
    (ret << ... << args);
    return ret.getBytes();
}

} // namespace


namespace lk
{

//=====================================

Peer::Handler::Handler(Peer& owning_peer, Network& owning_network_object, net::Session& handled_session, Core& core)
    : _owning_peer{owning_peer}, _owning_network_object{owning_network_object}, _session{handled_session}, _core{core}
{
    _session.start();
}


void Peer::Handler::onReceive(const base::Bytes& data)
{
    if(_session.isClosed()) {
        return;
    }

    base::SerializationIArchive ia(data);
    MessageType type;
    ia >> type;

    if(_owning_peer._state != State::SYNCHRONISED) {
        if(type == MessageType::HANDSHAKE) {
            LOG_DEBUG << "RECEIVED HANDSHAKE";
            auto top_block = bc::Block::deserialize(ia);
            std::optional<std::uint16_t> public_port;
            ia >> public_port;
            onHandshakeMessage(std::move(top_block), std::move(public_port));
        }
        else if(type == MessageType::BLOCK) {
            LOG_DEBUG << "RECEIVED SYNCHRONIZATION BLOCK";
            bc::Block block = bc::Block::deserialize(ia);
            onBlockMessage(std::move(block));
        }
        else {
            LOG_DEBUG << "Received on non-handshaked connection with peer " << _session.getId();
        }
    }
    else {
        LOG_DEBUG << "RECEIVED " << enumToString(type);
        switch(type) {
            case MessageType::PING: {
                onPingMessage();
                break;
            }
            case MessageType::PONG: {
                onPongMessage();
                break;
            }
            case MessageType::TRANSACTION: {
                bc::Transaction tx;
                ia >> tx;
                onTransactionMessage(std::move(tx));
                break;
            }
            case MessageType::BLOCK: {
                auto block = bc::Block::deserialize(ia);
                onBlockMessage(std::move(block));
                break;
            }
            case MessageType::GET_BLOCK: {
                base::Bytes block_hash;
                ia >> block_hash;
                onGetBlockMessage(base::Sha256(std::move(block_hash)));
                break;
            }
            case MessageType::INFO: {
                base::Bytes top_block_hash;
                std::vector<std::string> available_peers_strings;
                ia >> top_block_hash >> available_peers_strings;
                std::vector<net::Endpoint> available_peers;
                for(auto&& s: available_peers_strings) {
                    available_peers.emplace_back(std::move(s));
                }
                onInfoMessage(std::move(top_block_hash), std::move(available_peers));
                break;
            }
            case MessageType::GET_INFO: {
                onGetInfoMessage();
                break;
            }
            default: {
                LOG_DEBUG << "Received an invalid block from peer " << _session.getId()
                          << " with msgtype = " << static_cast<int>(type);
                break;
            }
        }
    }
}


void Peer::Handler::onClose()
{
    LOG_DEBUG << "Closing peer";
    _owning_network_object.removePeer(_owning_peer);
}


void Peer::Handler::onHandshakeMessage(bc::Block&& theirs_top_block, std::optional<std::uint16_t>&& public_port)
{
    const auto& ours_top_block = _core.getTopBlock();

    if(public_port) {
        net::Endpoint ep = _session.getEndpoint();
        ep.setPort(*public_port);
        _owning_peer.setServerEndpoint(std::move(ep));
    }
    else {
        _owning_peer.setServerEndpoint(_session.getEndpoint());
    }

    if(theirs_top_block == ours_top_block) {
        _owning_peer._state = State::SYNCHRONISED;
        return; // nothing changes, because top blocks are equal
    }
    else {
        if(ours_top_block.getDepth() > theirs_top_block.getDepth()) {
            _owning_peer._state = State::SYNCHRONISED;
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            if(_core.getTopBlock().getDepth() + 1 == theirs_top_block.getDepth()) {
                _core.tryAddBlock(theirs_top_block);
                _owning_peer._state = State::SYNCHRONISED;
            }
            else {
                _session.send(createMessage(MessageType::GET_BLOCK, theirs_top_block.getPrevBlockHash().getBytes()));
                _owning_peer._state = State::REQUESTED_BLOCKS;
                _sync_blocks.emplace_front(std::move(theirs_top_block));
            }
        }
    }
}


void Peer::Handler::onPingMessage()
{
    _session.send(createMessage(MessageType::PONG));
}


void Peer::Handler::onPongMessage()
{}


void Peer::Handler::onTransactionMessage(bc::Transaction&& tx)
{
    _core.performTransaction(tx);
}


void Peer::Handler::onBlockMessage(bc::Block&& block)
{
    if(_owning_peer._state == State::SYNCHRONISED) {
        _core.tryAddBlock(block);
    }
    else {
        _sync_blocks.emplace_front(std::move(block));

        if(block.getDepth() == _core.getTopBlock().getDepth() + 1) {
            // all requested blocks are received
            for(auto&& x: _sync_blocks) {
                if(!_core.tryAddBlock(x)) {
                    return; // TODO: reduce peer rating
                }
            }
            _sync_blocks.clear();
        }
        else {
            _session.send(createMessage(MessageType::GET_BLOCK, _sync_blocks.front().getPrevBlockHash().getBytes()));
        }
    }
}


void Peer::Handler::onGetBlockMessage(base::Sha256&& block_hash)
{
    auto block = _core.findBlock(block_hash);
    if(block) {
        _session.send(createMessage(MessageType::BLOCK, *block));
    }
    else {
        _session.send(createMessage(MessageType::BLOCK_NOT_FOUND, block_hash.getBytes()));
    }
}


void Peer::Handler::onGetInfoMessage()
{
    _session.send(createMessage(MessageType::INFO, _core.getTopBlock(), _owning_network_object.allPeersAddresses()));
}


void Peer::Handler::onInfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers)
{}

//=====================================

Peer::Peer(Network& owning_network_object, net::Session& session, Core& core)
    : _owning_network_object{owning_network_object}, _session{session}, _core{core}
{
    doHandshake();
}


std::unique_ptr<net::Handler> Peer::createHandler()
{
    return std::make_unique<Peer::Handler>(*this, _owning_network_object, _session, _core);
}


std::optional<net::Endpoint> Peer::getServerEndpoint() const
{
    return _address_for_incoming_connections;
}


void Peer::setServerEndpoint(net::Endpoint endpoint)
{
    _address_for_incoming_connections = std::move(endpoint);
}


void Peer::doHandshake()
{
    _session.send(createMessage(MessageType::HANDSHAKE, _core.getTopBlock(), _owning_network_object._public_port));
}

//=====================================

Network::HandlerFactory::HandlerFactory(Network& owning_network_object) : _owning_network_object{owning_network_object}
{}


std::unique_ptr<net::Handler> Network::HandlerFactory::create(net::Session& session)
{
    auto& peer = _owning_network_object.createPeer(session);
    return peer.createHandler();
}


void Network::HandlerFactory::destroy()
{}

//=====================================

Network::Network(const base::PropertyTree& config, Core& core) : _config{config}, _host{_config}, _core{core}
{
    if(_config.hasKey("net.public_port")) {
        _public_port = _config.get<std::uint16_t>("net.public_port");
    }
}


Peer& Network::createPeer(net::Session& session)
{
    auto& peer = _peers.emplace_front(*this, session, _core);
    return peer;
}


void Network::removePeer(const Peer& peer)
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


std::vector<net::Endpoint> Network::allPeersAddresses() const
{
    std::vector<net::Endpoint> ret;
    for(const auto& peer: _peers) {
        if(auto server_endpoint = peer.getServerEndpoint()) {
            ret.push_back(*std::move(server_endpoint));
        }
    }
    return ret;
}


void Network::run()
{
    _host.run(std::make_unique<HandlerFactory>(*this));
}

} // namespace lk