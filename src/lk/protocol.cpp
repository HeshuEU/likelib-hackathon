#include "protocol.hpp"

#include "base/log.hpp"
#include "lk/core.hpp"


namespace
{

template<typename M, typename... Args>
base::Bytes serializeMessage(Args&&... args)
{
    LOG_TRACE << lk::enumToString(M::getHandledMessageType());
    base::SerializationOArchive oa;
    oa.serialize(M::getHandledMessageType());
    //(oa << ... << std::forward<Args>(args));
    (oa.serialize(std::forward<Args>(args)),...);
    return std::move(oa).getBytes();
}

} // namespace


namespace lk
{

//============================================

PeerInfo PeerInfo::deserialize(base::SerializationIArchive& ia)
{
    auto endpoint = net::Endpoint::deserialize(ia);
    base::Bytes address = ia.deserialize<base::Bytes>();
    return {std::move(endpoint), std::move(address)};
}


void PeerInfo::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(endpoint);
    oa.serialize(address);
}

//============================================

constexpr MessageType HandshakeMessage::getHandledMessageType()
{
    return MessageType::HANDSHAKE;
}


void HandshakeMessage::serialize(base::SerializationOArchive& oa, const bc::Block& block, const base::Bytes& address,
    std::uint16_t public_port, const std::vector<PeerInfo>& known_peers)
{
    oa.serialize(MessageType::HANDSHAKE);
    oa.serialize(block);
    oa.serialize(address);
    oa.serialize(public_port);
    oa.serialize(known_peers);
}


void HandshakeMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _theirs_top_block, _address, _public_port, _known_peers);
}


HandshakeMessage HandshakeMessage::deserialize(base::SerializationIArchive& ia)
{
    auto top_block = ia.deserialize<bc::Block>();
    base::Bytes address = ia.deserialize<base::Bytes>();
    std::uint16_t public_port = ia.deserialize<std::uint16_t>();
    auto known_peers = ia.deserialize<std::vector<PeerInfo>>();
    return HandshakeMessage(std::move(top_block), std::move(address), public_port, std::move(known_peers));
}


void HandshakeMessage::handle(Peer& peer, Network& network, Core& core)
{
    const auto& ours_top_block = core.getTopBlock();

    if(auto ep = peer.getPublicEndpoint(); !ep && _public_port) {
        auto public_ep = peer.getEndpoint();
        public_ep.setPort(_public_port);
        peer.setServerEndpoint(public_ep);
    }

    for(const auto& peer_info: _known_peers) {
        network.checkOutNode(peer_info.endpoint, peer_info.address);
    }

    if(_theirs_top_block == ours_top_block) {
        peer.setState(Peer::State::SYNCHRONISED);
        return; // nothing changes, because top blocks are equal
    }
    else {
        if(ours_top_block.getDepth() > _theirs_top_block.getDepth()) {
            peer.setState(Peer::State::SYNCHRONISED);
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            if(core.getTopBlock().getDepth() + 1 == _theirs_top_block.getDepth()) {
                core.tryAddBlock(_theirs_top_block);
                peer.setState(Peer::State::SYNCHRONISED);
            }
            else {
                peer.addSyncBlock(std::move(_theirs_top_block));
                base::SerializationOArchive oa;
                GetBlockMessage::serialize(oa, _theirs_top_block.getPrevBlockHash());
                peer.send(std::move(oa).getBytes());
                peer.setState(Peer::State::REQUESTED_BLOCKS);
            }
        }
    }
}


HandshakeMessage::HandshakeMessage(
    bc::Block&& top_block, base::Bytes address, std::uint16_t public_port, std::vector<PeerInfo>&& known_peers)
    : _theirs_top_block{std::move(top_block)}, _address{std::move(address)}, _public_port{public_port},
      _known_peers{std::move(known_peers)}
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
    oa.serialize(MessageType::TRANSACTION);
    oa.serialize(tx);
}


void TransactionMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _tx);
}


TransactionMessage TransactionMessage::deserialize(base::SerializationIArchive& ia)
{
    auto tx = bc::Transaction::deserialize(ia);
    return {std::move(tx)};
}


void TransactionMessage::handle(Peer& peer, Network& network, Core& core)
{
    core.performTransaction(_tx);
}


TransactionMessage::TransactionMessage(bc::Transaction tx) : _tx{std::move(tx)}
{}

//============================================

constexpr MessageType GetBlockMessage::getHandledMessageType()
{
    return MessageType::GET_BLOCK;
}


void GetBlockMessage::serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash)
{
    oa.serialize(MessageType::GET_BLOCK);
    block_hash.serialize(oa);
}


void GetBlockMessage::serialize(base::SerializationOArchive& oa) const
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
    auto block = core.findBlock(_block_hash);
    if(block) {
        peer.send(serializeMessage<BlockMessage>(*block));
    }
    else {
        peer.send(serializeMessage<BlockNotFoundMessage>(_block_hash));
    }
}


GetBlockMessage::GetBlockMessage(base::Sha256 block_hash) : _block_hash{std::move(block_hash)}
{}

//============================================

constexpr MessageType BlockMessage::getHandledMessageType()
{
    return MessageType::BLOCK;
}


void BlockMessage::serialize(base::SerializationOArchive& oa, const bc::Block& block)
{
    oa.serialize(MessageType::BLOCK);
    oa.serialize(block);
}


void BlockMessage::serialize(base::SerializationOArchive& oa) const
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
            peer.send(serializeMessage<GetBlockMessage>(peer.getSyncBlocks().front().getPrevBlockHash()));
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


void BlockNotFoundMessage::serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash)
{
    oa.serialize(MessageType::BLOCK_NOT_FOUND);
    block_hash.serialize(oa);
}


void BlockNotFoundMessage::serialize(base::SerializationOArchive& oa) const
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
    oa.serialize(MessageType::GET_INFO);
}


GetInfoMessage GetInfoMessage::deserialize(base::SerializationIArchive& ia)
{
    return {};
}


void GetInfoMessage::handle(Peer& peer, Network& network, Core& core)
{
    peer.send(serializeMessage<InfoMessage>(core.getTopBlock(), network.allConnectedPeersInfo()));
}

//============================================

constexpr MessageType InfoMessage::getHandledMessageType()
{
    return MessageType::INFO;
}


void InfoMessage::serialize(base::SerializationOArchive& oa, const base::Sha256& top_block_hash,
    const std::vector<net::Endpoint>& available_peers)
{
    oa.serialize(MessageType::INFO);
    oa.serialize(top_block_hash);
    oa.serialize(available_peers);
}


void InfoMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _top_block_hash, _available_peers);
}


InfoMessage InfoMessage::deserialize(base::SerializationIArchive& ia)
{
    auto top_block_hash = ia.deserialize<base::Bytes>();
    auto available_peers = ia.deserialize<std::vector<net::Endpoint>>();
    return {std::move(top_block_hash), std::move(available_peers)};
}


void InfoMessage::handle(Peer& peer, Network& network, Core& core)
{}


InfoMessage::InfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers)
    : _top_block_hash{std::move(top_block_hash)}, _available_peers{std::move(available_peers)}
{}

//============================================

constexpr MessageType NewNodeMessage::getHandledMessageType()
{
    return MessageType::INFO;
}


void NewNodeMessage::serialize(
    base::SerializationOArchive& oa, const net::Endpoint& new_node_endpoint, const base::Bytes& address)
{
    oa.serialize(MessageType::NEW_NODE);
    oa.serialize(new_node_endpoint);
    oa.serialize(address);
}


void NewNodeMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _new_node_endpoint, _address);
}


NewNodeMessage NewNodeMessage::deserialize(base::SerializationIArchive& ia)
{
    auto ep = net::Endpoint::deserialize(ia);
    base::Bytes address = ia.deserialize<base::Bytes>();
    return {std::move(ep), std::move(address)};
}


void NewNodeMessage::handle(Peer& peer, Network& network, Core& core)
{
    if(network.checkOutNode(_new_node_endpoint, _address)) {
        network.broadcast(serializeMessage<NewNodeMessage>(_new_node_endpoint));
    }
}


NewNodeMessage::NewNodeMessage(net::Endpoint&& new_node_endpoint, base::Bytes&& address)
    : _new_node_endpoint{std::move(new_node_endpoint)}, _address{std::move(address)}
{}

//============================================

MessageProcessor::MessageProcessor(Peer& peer, Network& network, Core& core)
    : _peer{peer}, _network{network}, _core{core}
{}


namespace
{
    template<typename F, typename... O>
    void runHandleImpl(MessageType mt, base::SerializationIArchive& ia, Peer& peer, Network& network, Core& core)
    {
        if(F::getHandledMessageType() == mt) {
            auto message = F::deserialize(ia);
            message.handle(peer, network, core);
        }
        else {
            runHandleImpl<O...>(mt, ia, peer, network, core);
        }
    }


    template<>
    void runHandleImpl<void>(MessageType mt, base::SerializationIArchive& ia, Peer& peer, Network& network, Core& core)
    {}


    template<typename... Args>
    void runHandle(MessageType mt, base::SerializationIArchive& ia, Peer& peer, Network& network, Core& core,
        base::TypeList<Args...> type_list)
    {
        runHandleImpl<Args..., void>(mt, ia, peer, network, core);
    }
} // namespace


void MessageProcessor::process(const base::Bytes& raw_message)
{
    base::SerializationIArchive ia(raw_message);
    MessageType mt;
    ia >> mt;
    LOG_DEBUG << "Processing " << enumToString(mt) << " message";
    runHandle(mt, ia, _peer, _network, _core, _all_message_types);
    LOG_DEBUG << "Processed  " << enumToString(mt) << " message";
}

//============================================

Peer::Handler::Handler(Peer& owning_peer, Network& owning_network_object, net::Session& handled_session, Core& core)
    : _owning_peer{owning_peer}, _owning_network_object{owning_network_object}, _session{handled_session}, _core{core},
      _message_processor{_owning_peer, _owning_network_object, _core}
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


net::Endpoint Peer::getEndpoint() const
{
    return _session.getEndpoint();
}


std::optional<net::Endpoint> Peer::getPublicEndpoint() const
{
    return _endpoint_for_incoming_connections;
}


void Peer::setServerEndpoint(net::Endpoint endpoint)
{
    _endpoint_for_incoming_connections = std::move(endpoint);
}


void Peer::doHandshake()
{
    base::SerializationOArchive oa;
    std::uint16_t public_port = _owning_network_object._public_port ? *_owning_network_object._public_port : 0;
    auto connected_peers_info = _owning_network_object.allConnectedPeersInfo();
    HandshakeMessage::serialize(oa, _core.getTopBlock(), _core.getThisNodeAddress(), public_port, connected_peers_info);
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
    for(auto&& block: _sync_blocks) {
        if(!_core.tryAddBlock(block)) {
            return false;
        }
    }
    return true;
}


const std::forward_list<bc::Block>& Peer::getSyncBlocks() const noexcept
{
    return _sync_blocks;
}


void Peer::send(const base::Bytes& data)
{
    _session.send(data);
}


void Peer::send(base::Bytes&& data)
{
    _session.send(std::move(data));
}


std::optional<base::Bytes> Peer::getAddress() const
{
    return _address;
}


void Peer::setAddress(base::Bytes address)
{
    _address = std::move(address);
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
    _core.subscribeToBlockAddition(std::bind(&Network::onNewBlock, this, std::placeholders::_1));
    _core.subscribeToNewPendingTransaction(std::bind(&Network::onNewPendingTransaction, this, std::placeholders::_1));
}


Peer& Network::createPeer(net::Session& session)
{
    auto& peer = _peers.emplace_front(*this, session, _core);
    return peer;
}


void Network::removePeer(const Peer& peer)
{}


std::vector<PeerInfo> Network::allConnectedPeersInfo() const
{
    std::vector<PeerInfo> ret;
    for(const auto& peer: _peers) {
        if(auto server_endpoint = peer.getPublicEndpoint()) {
            if(auto address = peer.getAddress()) {
                ret.push_back(PeerInfo{*std::move(server_endpoint), *std::move(address)});
            }
        }
    }
    return ret;
}


bool Network::checkOutNode(const net::Endpoint& endpoint, const base::Bytes& address)
{
    if(_core.getThisNodeAddress() == address) {
        return false;
    }

    if(_host.isConnectedTo(endpoint)) {
        return false;
    }

    for(const auto& peer: _peers) {
        if(auto ep = peer.getPublicEndpoint(); ep && *ep == endpoint) {
            return false;
        }
    }

    LOG_DEBUG << "Connecting to node after Network::checkOutNode()";
    _host.connect(endpoint);
    LOG_DEBUG << "Connection to " << endpoint << " is added to queue";
    return true;
}


void Network::broadcast(const base::Bytes& data)
{
    LOG_DEBUG << "Broadcasting data size = " << data.size();
    _host.broadcast(data);
}


void Network::onNewBlock(const bc::Block& block)
{
    broadcast(serializeMessage<BlockMessage>(block));
}


void Network::onNewPendingTransaction(const bc::Transaction& tx)
{
    broadcast(serializeMessage<TransactionMessage>(tx));
}


void Network::run()
{
    _host.run(std::make_unique<HandlerFactory>(*this));

    if(_config.hasKey("nodes")) {
        for(const auto& node: _config.getVector<std::string>("nodes")) {
            _host.connect(net::Endpoint(node));
        }
    }
}

} // namespace lk