#include "protocol.hpp"

#include "base/log.hpp"
#include "lk/core.hpp"


namespace
{

template<typename M, typename... Args>
base::Bytes serializeMessage(Args&&... args)
{
    base::SerializationOArchive oa;
    oa << M::getHandledMessageType();
    (oa << ... << std::forward<Args>(args));
    return std::move(oa).getBytes();
}

} // namespace


namespace lk
{

//============================================

constexpr MessageType HandshakeMessage::getHandledMessageType()
{
    return MessageType::HANDSHAKE;
}


void HandshakeMessage::serialize(base::SerializationOArchive& oa, const bc::Block& block, std::uint16_t public_port,
    const std::vector<net::Endpoint>& known_endpoints)
{
    oa << MessageType::HANDSHAKE << block << public_port << known_endpoints;
}


void HandshakeMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _theirs_top_block, _public_port, _known_endpoints);
}


HandshakeMessage HandshakeMessage::deserialize(base::SerializationIArchive& ia)
{
    auto top_block = bc::Block::deserialize(ia);
    std::uint16_t public_port;
    ia >> public_port;
    std::vector<net::Endpoint> known_endpoints;
    ia >> known_endpoints;
    return HandshakeMessage(std::move(top_block), public_port, std::move(known_endpoints));
}


void HandshakeMessage::handle(Peer& peer, Network& network, Core& core)
{
    const auto& ours_top_block = core.getTopBlock();

    if(auto ep = peer.getPublicEndpoint(); !ep && _public_port) {
        auto public_ep = peer.getEndpoint();
        public_ep.setPort(_public_port);
        peer.setServerEndpoint(public_ep);
    }

    for(const auto& endpoint : _known_endpoints) {
        network.checkOutNode(endpoint);
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


HandshakeMessage::HandshakeMessage(bc::Block&& top_block, std::uint16_t public_port, std::vector<net::Endpoint>&& known_endpoints)
    : _theirs_top_block{std::move(top_block)}, _public_port{public_port}, _known_endpoints{std::move(known_endpoints)}
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


void TransactionMessage::serialize(base::SerializationOArchive& oa) const
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
    oa << MessageType::GET_BLOCK;
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
    oa << MessageType::BLOCK << block;
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
    oa << MessageType::BLOCK_NOT_FOUND;
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
    oa << MessageType::GET_INFO;
}


GetInfoMessage GetInfoMessage::deserialize(base::SerializationIArchive& ia)
{
    return {};
}


void GetInfoMessage::handle(Peer& peer, Network& network, Core& core)
{
    peer.send(serializeMessage<InfoMessage>(core.getTopBlock(), network.allPeersAddresses()));
}

//============================================

constexpr MessageType InfoMessage::getHandledMessageType()
{
    return MessageType::INFO;
}


void InfoMessage::serialize(base::SerializationOArchive& oa, const base::Sha256& top_block_hash,
    const std::vector<net::Endpoint>& available_peers)
{
    oa << MessageType::INFO;
    top_block_hash.serialize(oa);
    oa << available_peers;
}


void InfoMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _top_block_hash, _available_peers);
}


InfoMessage InfoMessage::deserialize(base::SerializationIArchive& ia)
{
    base::Bytes top_block_hash;
    std::vector<net::Endpoint> available_peers;
    ia >> top_block_hash >> available_peers;
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


void NewNodeMessage::serialize(base::SerializationOArchive& oa, const net::Endpoint& new_node_endpoint)
{
    oa << MessageType::NEW_NODE << new_node_endpoint;
}


void NewNodeMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _new_node_endpoint);
}


NewNodeMessage NewNodeMessage::deserialize(base::SerializationIArchive& ia)
{
    auto ep = net::Endpoint::deserialize(ia);
    return {std::move(ep)};
}


void NewNodeMessage::handle(Peer& peer, Network& network, Core& core)
{
    if(network.checkOutNode(_new_node_endpoint)) {
        network.broadcast(serializeMessage<NewNodeMessage>(_new_node_endpoint));
    }
}


NewNodeMessage::NewNodeMessage(net::Endpoint&& new_node_endpoint) : _new_node_endpoint{std::move(new_node_endpoint)}
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
    {
        return;
    }


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
    return _address_for_incoming_connections;
}


void Peer::setServerEndpoint(net::Endpoint endpoint)
{
    _address_for_incoming_connections = std::move(endpoint);
}


void Peer::doHandshake()
{
    base::SerializationOArchive oa;
    std::uint16_t public_port = _owning_network_object._public_port ? *_owning_network_object._public_port : 0;
    HandshakeMessage::serialize(oa, _core.getTopBlock(), public_port, _owning_network_object.allPeersAddresses());
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


std::vector<net::Endpoint> Network::allPeersAddresses() const
{
    std::vector<net::Endpoint> ret;
    for(const auto& peer: _peers) {
        if(auto server_endpoint = peer.getPublicEndpoint()) {
            ret.push_back(*std::move(server_endpoint));
        }
    }
    return ret;
}


bool Network::checkOutNode(const net::Endpoint& endpoint)
{
    if(_host.isConnectedTo(endpoint)) {
        return false;
    }

    for(const auto& peer : _peers) {
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
    _host.broadcast(data);
}


void Network::onNewBlock(const bc::Block& block)
{
    LOG_DEBUG << "Network::onNewBlock()";
    broadcast(serializeMessage<BlockMessage>(block));
}


void Network::onNewPendingTransaction(const bc::Transaction& tx)
{
    LOG_DEBUG << "Network::onNewPendingTransaction()";
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