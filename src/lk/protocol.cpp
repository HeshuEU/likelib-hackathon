#include "protocol.hpp"

#include "base/log.hpp"
#include "lk/core.hpp"


namespace
{

// clang-format off
DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(MessageType, std::uint8_t,
                                          (NOT_AVAILABLE)
                                            (HANDSHAKE)
                                            (PING)
                                            (PONG)
                                            (TRANSACTION)
                                            (GET_BLOCK)
                                            (BLOCK)
                                            (BLOCK_NOT_FOUND)
                                            (GET_INFO)
                                            (INFO)
                                            (NEW_NODE)
)
// clang-format on


template<typename M, typename... Args>
base::Bytes serializeMessage(Args&&... args)
{
    LOG_TRACE << enumToString(M::getHandledMessageType());
    base::SerializationOArchive oa;
    oa.serialize(M::getHandledMessageType());
    (oa.serialize(std::forward<Args>(args)), ...);
    return std::move(oa).getBytes();
}

//========================================


struct PeerInfo
{
    net::Endpoint endpoint;
    bc::Address address;

    static PeerInfo deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
};


class HandshakeMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const bc::Block& block,
                          const bc::Address& address,
                          std::uint16_t public_port,
                          const std::vector<PeerInfo>& known_peers);
    void serialize(base::SerializationOArchive& oa) const;
    static HandshakeMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Block _theirs_top_block;
    bc::Address _address;
    std::uint16_t
      _public_port; // zero public port states that peer didn't provide information about his public endpoint
    std::vector<PeerInfo> _known_peers;

    HandshakeMessage(bc::Block&& top_block,
                     bc::Address address,
                     std::uint16_t public_port,
                     std::vector<PeerInfo>&& known_peers);
};


class PingMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PingMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    PingMessage() = default;
};

//============================================

class PongMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PongMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    PongMessage() = default;
};

//============================================

class TransactionMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, bc::Transaction tx);
    void serialize(base::SerializationOArchive& oa) const;
    static TransactionMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Transaction _tx;

    TransactionMessage(bc::Transaction tx);
};

//============================================

class GetBlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    void serialize(base::SerializationOArchive& oa) const;
    static GetBlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _block_hash;

    GetBlockMessage(base::Sha256 block_hash);
};

//============================================

class BlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const bc::Block& block);
    void serialize(base::SerializationOArchive& oa) const;
    static BlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Block _block;

    BlockMessage(bc::Block block);
};

//============================================

class BlockNotFoundMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    void serialize(base::SerializationOArchive& oa) const;
    static BlockNotFoundMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _block_hash;

    BlockNotFoundMessage(base::Sha256 block_hash);
};

//============================================

class GetInfoMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static GetInfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    GetInfoMessage() = default;
};

//============================================

class InfoMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const base::Sha256& top_block_hash,
                          const std::vector<net::Endpoint>& available_peers);
    void serialize(base::SerializationOArchive& oa) const;
    static InfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _top_block_hash;
    std::vector<net::Endpoint> _available_peers;

    InfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers);
};

//============================================

class NewNodeMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const net::Endpoint& new_node_endpoint,
                          const bc::Address& address);
    void serialize(base::SerializationOArchive& oa) const;
    static NewNodeMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    net::Endpoint _new_node_endpoint;
    bc::Address _address;

    NewNodeMessage(net::Endpoint&& new_node_endpoint, bc::Address&& address);
};

//========================================

} // namespace


namespace lk
{

//============================================

class MessageProcessor
{
  public:
    MessageProcessor(Peer& peer, Network& network, Core& core);

    void process(const base::Bytes& raw_message);

  private:
    static const base::TypeList<HandshakeMessage,
      PingMessage,
      PongMessage,
      TransactionMessage,
      GetBlockMessage,
      BlockMessage,
      BlockNotFoundMessage,
      GetInfoMessage,
      InfoMessage,
      NewNodeMessage>
      _all_message_types;

    Peer& _peer;
    Network& _network;
    Core& _core;
};

//============================================

PeerInfo PeerInfo::deserialize(base::SerializationIArchive& ia)
{
    auto endpoint = ia.deserialize<net::Endpoint>();
    auto address = ia.deserialize<bc::Address>();
    return { std::move(endpoint), std::move(address) };
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


void HandshakeMessage::serialize(base::SerializationOArchive& oa,
                                 const bc::Block& block,
                                 const bc::Address& address,
                                 std::uint16_t public_port,
                                 const std::vector<PeerInfo>& known_peers)
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
    auto address = ia.deserialize<bc::Address>();
    auto public_port = ia.deserialize<std::uint16_t>();
    auto known_peers = ia.deserialize<std::vector<PeerInfo>>();
    return HandshakeMessage(std::move(top_block), std::move(address), public_port, std::move(known_peers));
}


void HandshakeMessage::handle(Peer& peer, Network& network, Core& core)
{
    const auto& ours_top_block = core.getTopBlock();

    if (auto ep = peer.getPublicEndpoint(); !ep && _public_port) {
        auto public_ep = peer.getEndpoint();
        public_ep.setPort(_public_port);
        peer.setServerEndpoint(public_ep);
    }

    for (const auto& peer_info : _known_peers) {
        network.checkOutNode(peer_info.endpoint, peer_info.address);
    }

    if (_theirs_top_block == ours_top_block) {
        peer.setState(Peer::State::SYNCHRONISED);
        return; // nothing changes, because top blocks are equal
    }
    else {
        if (ours_top_block.getDepth() > _theirs_top_block.getDepth()) {
            peer.setState(Peer::State::SYNCHRONISED);
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            if (core.getTopBlock().getDepth() + 1 == _theirs_top_block.getDepth()) {
                core.tryAddBlock(_theirs_top_block);
                peer.setState(Peer::State::SYNCHRONISED);
            }
            else {
                base::SerializationOArchive oa;
                GetBlockMessage::serialize(oa, _theirs_top_block.getPrevBlockHash());
                peer.send(std::move(oa).getBytes());
                peer.setState(Peer::State::REQUESTED_BLOCKS);
                peer.addSyncBlock(std::move(_theirs_top_block));
            }
        }
    }
}


HandshakeMessage::HandshakeMessage(bc::Block&& top_block,
                                   bc::Address address,
                                   std::uint16_t public_port,
                                   std::vector<PeerInfo>&& known_peers)
  : _theirs_top_block{ std::move(top_block) }
  , _address{ std::move(address) }
  , _public_port{ public_port }
  , _known_peers{ std::move(known_peers) }
{}

//============================================

constexpr MessageType PingMessage::getHandledMessageType()
{
    return MessageType::PING;
}


void PingMessage::serialize(base::SerializationOArchive& oa)
{
    oa.serialize(MessageType::PING);
}


PingMessage PingMessage::deserialize(base::SerializationIArchive& ia)
{
    return {};
}


void PingMessage::handle(Peer& peer, Network& network, Core& core) {}


//============================================

constexpr MessageType PongMessage::getHandledMessageType()
{
    return MessageType::PONG;
}


void PongMessage::serialize(base::SerializationOArchive& oa)
{
    oa.serialize(MessageType::PONG);
}


PongMessage PongMessage::deserialize(base::SerializationIArchive& ia)
{
    return {};
}


void PongMessage::handle(Peer& peer, Network& network, Core& core) {}

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
    return { std::move(tx) };
}


void TransactionMessage::handle(Peer& peer, Network& network, Core& core)
{
    core.addPendingTransaction(_tx);
}


TransactionMessage::TransactionMessage(bc::Transaction tx)
  : _tx{ std::move(tx) }
{}

//============================================

constexpr MessageType GetBlockMessage::getHandledMessageType()
{
    return MessageType::GET_BLOCK;
}


void GetBlockMessage::serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash)
{
    oa.serialize(MessageType::GET_BLOCK);
    oa.serialize(block_hash);
}


void GetBlockMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _block_hash);
}


GetBlockMessage GetBlockMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return { std::move(block_hash) };
}


void GetBlockMessage::handle(Peer& peer, Network& network, Core& core)
{
    auto block = core.findBlock(_block_hash);
    if (block) {
        peer.send(serializeMessage<BlockMessage>(*block));
    }
    else {
        peer.send(serializeMessage<BlockNotFoundMessage>(_block_hash));
    }
}


GetBlockMessage::GetBlockMessage(base::Sha256 block_hash)
  : _block_hash{ std::move(block_hash) }
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
    return { std::move(block) };
}


void BlockMessage::handle(Peer& peer, Network& network, Core& core)
{
    if (peer.getState() == Peer::State::SYNCHRONISED) {
        core.tryAddBlock(std::move(_block));
    }
    else {
        bc::BlockDepth block_depth = _block.getDepth();
        peer.addSyncBlock(std::move(_block));

        if (block_depth == core.getTopBlock().getDepth() + 1) {
            peer.applySyncs();
        }
        else {
            peer.send(serializeMessage<GetBlockMessage>(peer.getSyncBlocks().front().getPrevBlockHash()));
        }
    }
}


BlockMessage::BlockMessage(bc::Block block)
  : _block{ std::move(block) }
{}

//============================================

constexpr MessageType BlockNotFoundMessage::getHandledMessageType()
{
    return MessageType::BLOCK_NOT_FOUND;
}


void BlockNotFoundMessage::serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash)
{
    oa.serialize(MessageType::BLOCK_NOT_FOUND);
    oa.serialize(block_hash);
}


void BlockNotFoundMessage::serialize(base::SerializationOArchive& oa) const
{
    serialize(oa, _block_hash);
}


BlockNotFoundMessage BlockNotFoundMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return { std::move(block_hash) };
}


void BlockNotFoundMessage::handle(Peer& peer, Network& network, Core& core) {}


BlockNotFoundMessage::BlockNotFoundMessage(base::Sha256 block_hash)
  : _block_hash{ std::move(block_hash) }
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


void InfoMessage::serialize(base::SerializationOArchive& oa,
                            const base::Sha256& top_block_hash,
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
    return { std::move(top_block_hash), std::move(available_peers) };
}


void InfoMessage::handle(Peer& peer, Network& network, Core& core) {}


InfoMessage::InfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers)
  : _top_block_hash{ std::move(top_block_hash) }
  , _available_peers{ std::move(available_peers) }
{}

//============================================

constexpr MessageType NewNodeMessage::getHandledMessageType()
{
    return MessageType::INFO;
}


void NewNodeMessage::serialize(base::SerializationOArchive& oa,
                               const net::Endpoint& new_node_endpoint,
                               const bc::Address& address)
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
    auto address = ia.deserialize<bc::Address>();
    return { std::move(ep), std::move(address) };
}


void NewNodeMessage::handle(Peer& peer, Network& network, Core& core)
{
    if (network.checkOutNode(_new_node_endpoint, _address)) {
        network.broadcast(serializeMessage<NewNodeMessage>(_new_node_endpoint));
    }
}


NewNodeMessage::NewNodeMessage(net::Endpoint&& new_node_endpoint, bc::Address&& address)
  : _new_node_endpoint{ std::move(new_node_endpoint) }
  , _address{ std::move(address) }
{}

//============================================

MessageProcessor::MessageProcessor(Peer& peer, Network& network, Core& core)
  : _peer{ peer }
  , _network{ network }
  , _core{ core }
{}


namespace
{
template<typename F, typename... O>
void runHandleImpl(MessageType mt, base::SerializationIArchive& ia, Peer& peer, Network& network, Core& core)
{
    if (F::getHandledMessageType() == mt) {
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
void runHandle(MessageType mt,
               base::SerializationIArchive& ia,
               Peer& peer,
               Network& network,
               Core& core,
               base::TypeList<Args...> type_list)
{
    runHandleImpl<Args..., void>(mt, ia, peer, network, core);
}
} // namespace


void MessageProcessor::process(const base::Bytes& raw_message)
{
    base::SerializationIArchive ia(raw_message);
    auto mt = ia.deserialize<MessageType>();
    LOG_DEBUG << "Processing " << enumToString(mt) << " message";
    runHandle(mt, ia, _peer, _network, _core, _all_message_types);
    LOG_DEBUG << "Processed  " << enumToString(mt) << " message";
}

//============================================

Network::Network(const base::PropertyTree& config, Core& core)
  : _config{ config }
  , _host{ _config, config.get<std::size_t>("net.max_connections") }
  , _core{ core }
{
    if (_config.hasKey("net.public_port")) {
        _public_port = _config.get<std::uint16_t>("net.public_port");
    }
    _core.subscribeToBlockAddition(std::bind(&Network::onNewBlock, this, std::placeholders::_1));
    _core.subscribeToNewPendingTransaction(std::bind(&Network::onNewPendingTransaction, this, std::placeholders::_1));
}



std::vector<PeerInfo> Network::allConnectedPeersInfo() const
{
    std::vector<PeerInfo> ret;
    for (const auto& peer : _peers) {
        if (auto server_endpoint = peer.getPublicEndpoint()) {
            if (auto address = peer.getAddress()) {
                ret.push_back(PeerInfo{ *std::move(server_endpoint), std::move(*address) });
            }
        }
    }
    return ret;
}


void Network::onNewBlock(const bc::Block& block)
{
    broadcast(serializeMessage<BlockMessage>(block));
}


void Network::onNewPendingTransaction(const bc::Transaction& tx)
{
    broadcast(serializeMessage<TransactionMessage>(tx));
}


} // namespace lk
