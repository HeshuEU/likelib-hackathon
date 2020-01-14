#include "protocol.hpp"

#include "base/log.hpp"
#include "lk/core.hpp"


namespace lk
{

//============================================

constexpr MessageType HandshakeMessage::getHandledMessageType()
{
    return MessageType::HANDSHAKE;
}


void HandshakeMessage::serialize(base::SerializationOArchive& oa, const bc::Block& block, std::uint16_t public_port)
{
    oa << MessageType::HANDSHAKE << block << public_port;
}


void HandshakeMessage::serialize(base::SerializationOArchive& oa)
{
    serialize(oa, _top_block, _public_port);
}


HandshakeMessage HandshakeMessage::deserialize(base::SerializationIArchive& ia)
{
    auto top_block = bc::Block::deserialize(ia);
    std::uint16_t _public_port;
    ia >> _public_port;
    return HandshakeMessage(std::move(top_block), _public_port);
}


void HandshakeMessage::handle(Peer& peer, Network& network, Core& core)
{
    const auto& ours_top_block = core.getTopBlock();

    if(auto ep = peer.getServerEndpoint(); !ep) {
        peer.setServerEndpoint(std::move(*ep));
    }

    if(_top_block == ours_top_block) {
        peer.setState(Peer::State::SYNCHRONISED);
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


HandshakeMessage::HandshakeMessage(bc::Block&& top_block, std::uint16_t public_port)
    : _top_block{std::move(top_block)}, _public_port{public_port}
{}

//============================================

constexpr MessageType PingMessage::getHandledMessageType()
{
    return MessageType::PING;
}


void PingMessage::serialize(base::SerializationOArchive& oa)
{
    oa << MessageType::PING;
}


PingMessage PingMessage::deserialize(base::SerializationIArchive& ia)
{
    return {};
}


void PingMessage::handle(Peer& peer, Network& network, Core& core)
{}


//============================================

constexpr MessageType PongMessage::getHandledMessageType()
{
    return MessageType::PONG;
}


void PongMessage::serialize(base::SerializationOArchive& oa)
{
    oa << MessageType::PONG;
}


PongMessage PongMessage::deserialize(base::SerializationIArchive& ia)
{
    return {};
}


void PongMessage::handle(Peer& peer, Network& network, Core& core)
{}

//============================================

constexpr MessageType TransactionMessage::getHandledMessageType()
{
    return MessageType::TRANSACTION;
}


void TransactionMessage::serialize(base::SerializationOArchive& oa, bc::Transaction tx)
{
    oa << MessageType::TRANSACTION << tx;
}


void TransactionMessage::serialize(base::SerializationOArchive& oa)
{
    serialize(oa, _tx);
}


TransactionMessage TransactionMessage::deserialize(base::SerializationIArchive& ia)
{
    bc::Transaction tx;
    ia >> tx;
    return {std::move(tx)};
}


void TransactionMessage::handle(Peer& peer, Network& network, Core& core)
{
    _core.performTransaction(tx);
}


TransactionMessage::TransactionMessage(bc::Transaction tx) : _tx{std::move(tx)}
{}

//============================================

constexpr MessageType GetBlockMessage::getHandledMessageType()
{
    return MessageType::GET_BLOCK;
}


void GetBlockMessage::serialize(base::SerializationOArchive& oa, base::Sha256& block_hash)
{
    oa << MessageType::GET_BLOCK;
    block_hash.serialize(oa);
}


void GetBlockMessage::serialize(base::SerializationOArchive& oa)
{
    serialize(oa, _block_hash);
}


GetBlockMessage GetBlockMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return {std::move(block_hash)};
}


void GetBlockMessage::handle(Peer& peer, Network& network, Core& core)
{
    auto block = _core.findBlock(block_hash);
    if(block) {
        _session.send(createMessage(MessageType::BLOCK, *block));
    }
    else {
        _session.send(createMessage(MessageType::BLOCK_NOT_FOUND, block_hash.getBytes()));
    }
}


GetBlockMessage::GetBlockMessage(base::Sha256 block_hash) : _block_hash{std::move(block_hash)}
{}

//============================================

constexpr MessageType BlockMessage::getHandledMessageType()
{
    return MessageType::BLOCK;
}


void BlockMessage::serialize(base::SerializationOArchive& oa, bc::Block& block)
{
    oa << MessageType::BLOCK << block;
}


void BlockMessage::serialize(base::SerializationOArchive& oa)
{
    serialize(oa, _block);
}


BlockMessage BlockMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block = bc::Block::deserialize(ia);
    return {std::move(block)};
}


void BlockMessage::handle(Peer& peer, Network& network, Core& core)
{
    if(peer.getState() == Peer::State::SYNCHRONISED) {
        core.tryAddBlock(std::move(_block));
    }
    else {
        bc::BlockDepth block_depth = _block.getDepth();
        peer.addSyncBlock(std::move(_block));

        if(block_depth == core.getTopBlock().getDepth() + 1) {
            peer.applySyncs();
        }
        else {
            peer.send(createMessage(MessageType::GET_BLOCK, _sync_blocks.front().getPrevBlockHash().getBytes()));
        }
    }
}


BlockMessage::BlockMessage(bc::Block block) : _block{std::move(block)}
{}

//============================================

constexpr MessageType BlockNotFoundMessage::getHandledMessageType()
{
    return MessageType::BLOCK_NOT_FOUND;
}


void BlockNotFoundMessage::serialize(base::SerializationOArchive& oa, base::Sha256& block_hash)
{
    oa << MessageType::BLOCK_NOT_FOUND;
    block_hash.serialize(oa);
}


void BlockNotFoundMessage::serialize(base::SerializationOArchive& oa)
{
    serialize(oa, _block_hash);
}


BlockNotFoundMessage BlockNotFoundMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return {std::move(block_hash)};
}


void BlockNotFoundMessage::handle(Peer& peer, Network& network, Core& core)
{}


BlockNotFoundMessage::BlockNotFoundMessage(base::Sha256 block_hash) : _block_hash{std::move(block_hash)}
{}

//============================================

constexpr MessageType GetInfoMessage::getHandledMessageType()
{
    return MessageType::GET_INFO;
}


void GetInfoMessage::serialize(base::SerializationOArchive& oa)
{
    oa << MessageType::GET_INFO;
}


GetInfoMessage GetInfoMessage::deserialize(base::SerializationIArchive& ia)
{
    return {};
}


void GetInfoMessage::handle(Peer& peer, Network& network, Core& core)
{
    peer.send(createMessage(MessageType::INFO, core.getTopBlock(), network.allPeersAddresses()));
}

//============================================

constexpr MessageType InfoMessage::getHandledMessageType()
{
    return MessageType::INFO;
}


void InfoMessage::serialize(
    base::SerializationOArchive& oa, base::Sha256& top_block_hash, std::vector<net::Endpoint>& available_peers)
{
    oa << MessageType::INFO;
    top_block_hash.serialize(oa);
    oa << available_peers;
}


void InfoMessage::serialize(base::SerializationOArchive& oa)
{
    serialize(oa, _top_block_hash, _available_peers);
}


InfoMessage InfoMessage::deserialize(base::SerializationIArchive& ia)
{
    base::Bytes top_block_hash;
    std::vector<std::string> available_peers_strings;
    ia >> top_block_hash >> available_peers_strings;
    std::vector<net::Endpoint> available_peers;
    for(auto&& s: available_peers_strings) {
        available_peers.emplace_back(std::move(s));
    }

    return {std::move(top_block_hash), std::move(available_peers)};
}


void InfoMessage::handle(Peer& peer, Network& network, Core& core)
{}

InfoMessage::InfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers)
    : _top_block_hash{std::move(top_block_hash)}, _available_peers{std::move(available_peers)}
{}

//============================================

void MessageProcessor::process(const base::Bytes& raw_message)
{}

//============================================

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

    _message_processor.process(data);
}


void Peer::Handler::onClose()
{
    LOG_DEBUG << "Closing peer";
    _owning_network_object.removePeer(_owning_peer);
}

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
    base::SerializationOArchive oa;
    HandshakeMessage::serialize(oa, _core.getTopBlock(), _owning_network_object._public_port ? *_owning_network_object._public_port : 0);
    _session.send(std::move(oa).getBytes());
}


void Peer::setState(State new_state)
{
    _state = new_state;
}


Peer::State Peer::getState() const noexcept
{
    return _state;
}


void Peer::addSyncBlock(bc::Block block)
{
    _sync_blocks.push_front(std::move(block));
}


bool Peer::applySyncs()
{
    for(auto&& block : _sync_blocks) {
        if(!_core.tryAddBlock(block)) {
            return false;
        }
    }
    return true;
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