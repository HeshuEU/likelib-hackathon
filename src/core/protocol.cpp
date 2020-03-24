#include "protocol.hpp"

#include "base/log.hpp"
#include "base/serialization.hpp"
#include "core/core.hpp"


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
    LOG_TRACE << "Serializing " << enumToString(M::getHandledMessageType());
    base::SerializationOArchive oa;
    oa.serialize(M::getHandledMessageType());
    (oa.serialize(std::forward<Args>(args)), ...);
    return std::move(oa).getBytes();
}

//========================================

class HandshakeMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const lk::Block& block,
                          const lk::Address& address,
                          std::uint16_t public_port,
                          const std::vector<lk::Peer::Info>& known_peers);
    static HandshakeMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::MessageProcessor::Context& context);

  private:
    lk::Block _theirs_top_block;
    lk::Address _address;
    std::uint16_t
      _public_port; // zero public port states that peer didn't provide information about his public endpoint
    std::vector<lk::Peer::Info> _known_peers;

    HandshakeMessage(lk::Block&& top_block,
                     lk::Address address,
                     std::uint16_t public_port,
                     std::vector<lk::Peer::Info>&& known_peers);
};

//========================================

class PingMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PingMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::MessageProcessor::Context& ctx);

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
    void handle(const lk::MessageProcessor::Context& ctx);

  private:
    PongMessage() = default;
};

//============================================

class TransactionMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, lk::Transaction tx);
    static TransactionMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::MessageProcessor::Context& ctx);

  private:
    lk::Transaction _tx;

    TransactionMessage(lk::Transaction tx);
};

//============================================

class GetBlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    static GetBlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::MessageProcessor::Context& ctx);

  private:
    base::Sha256 _block_hash;

    GetBlockMessage(base::Sha256 block_hash);
};

//============================================

class BlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const lk::Block& block);
    static BlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::MessageProcessor::Context& ctx);

  private:
    lk::Block _block;

    BlockMessage(lk::Block block);
};

//============================================

class BlockNotFoundMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    static BlockNotFoundMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::MessageProcessor::Context& ctx);

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
    void handle(const lk::MessageProcessor::Context& ctx);

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
    static InfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::MessageProcessor::Context& ctx);

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
                          const lk::Address& address);
    static NewNodeMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::MessageProcessor::Context& ctx);

  private:
    net::Endpoint _new_node_endpoint;
    lk::Address _address;

    NewNodeMessage(net::Endpoint&& new_node_endpoint, lk::Address&& address);
};

//============================================

constexpr MessageType HandshakeMessage::getHandledMessageType()
{
    return MessageType::HANDSHAKE;
}


void HandshakeMessage::serialize(base::SerializationOArchive& oa,
                                 const lk::Block& block,
                                 const lk::Address& address,
                                 std::uint16_t public_port,
                                 const std::vector<lk::Peer::Info>& known_peers)
{
    oa.serialize(MessageType::HANDSHAKE);
    oa.serialize(block);
    oa.serialize(address);
    oa.serialize(public_port);
    oa.serialize(known_peers);
}


HandshakeMessage HandshakeMessage::deserialize(base::SerializationIArchive& ia)
{
    auto top_block = ia.deserialize<lk::Block>();
    auto address = ia.deserialize<lk::Address>();
    auto public_port = ia.deserialize<std::uint16_t>();
    auto known_peers = ia.deserialize<std::vector<lk::Peer::Info>>();
    return HandshakeMessage(std::move(top_block), std::move(address), public_port, std::move(known_peers));
}


void HandshakeMessage::handle(const lk::MessageProcessor::Context& ctx)
{
    const auto& ours_top_block = ctx.core->getTopBlock();

    if (auto ep = ctx.peer->getPublicEndpoint(); !ep && _public_port) {
        auto public_ep = ctx.peer->getEndpoint();
        public_ep.setPort(_public_port);
        ctx.peer->setServerEndpoint(public_ep);
    }

    for (const auto& peer_info : _known_peers) {
        ctx.host->checkOutPeer(peer_info.endpoint);
    }

    if (_theirs_top_block == ours_top_block) {
        ctx.peer->setState(lk::Peer::State::SYNCHRONISED);
        return; // nothing changes, because top blocks are equal
    }
    else {
        if (ours_top_block.getDepth() > _theirs_top_block.getDepth()) {
            ctx.peer->setState(lk::Peer::State::SYNCHRONISED);
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            if (ctx.core->getTopBlock().getDepth() + 1 == _theirs_top_block.getDepth()) {
                ctx.core->tryAddBlock(_theirs_top_block);
                ctx.peer->setState(lk::Peer::State::SYNCHRONISED);
            }
            else {
                base::SerializationOArchive oa;
                GetBlockMessage::serialize(oa, _theirs_top_block.getPrevBlockHash());
                ctx.peer->send(std::move(oa).getBytes());
                ctx.peer->setState(lk::Peer::State::REQUESTED_BLOCKS);
                ctx.peer->addSyncBlock(std::move(_theirs_top_block));
            }
        }
    }
}


HandshakeMessage::HandshakeMessage(lk::Block&& top_block,
                                   lk::Address address,
                                   std::uint16_t public_port,
                                   std::vector<lk::Peer::Info>&& known_peers)
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


PingMessage PingMessage::deserialize(base::SerializationIArchive&)
{
    return {};
}


void PingMessage::handle(const lk::MessageProcessor::Context&) {}


//============================================

constexpr MessageType PongMessage::getHandledMessageType()
{
    return MessageType::PONG;
}


void PongMessage::serialize(base::SerializationOArchive& oa)
{
    oa.serialize(MessageType::PONG);
}


PongMessage PongMessage::deserialize(base::SerializationIArchive&)
{
    return {};
}


void PongMessage::handle(const lk::MessageProcessor::Context&) {}

//============================================

constexpr MessageType TransactionMessage::getHandledMessageType()
{
    return MessageType::TRANSACTION;
}


void TransactionMessage::serialize(base::SerializationOArchive& oa, lk::Transaction tx)
{
    oa.serialize(MessageType::TRANSACTION);
    oa.serialize(tx);
}


TransactionMessage TransactionMessage::deserialize(base::SerializationIArchive& ia)
{
    auto tx = lk::Transaction::deserialize(ia);
    return { std::move(tx) };
}


void TransactionMessage::handle(const lk::MessageProcessor::Context& ctx)
{
    ctx.core->addPendingTransaction(_tx);
}


TransactionMessage::TransactionMessage(lk::Transaction tx)
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


GetBlockMessage GetBlockMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return { std::move(block_hash) };
}


void GetBlockMessage::handle(const lk::MessageProcessor::Context& ctx)
{
    auto block = ctx.core->findBlock(_block_hash);
    if (block) {
        ctx.peer->send(serializeMessage<BlockMessage>(*block));
    }
    else {
        ctx.peer->send(serializeMessage<BlockNotFoundMessage>(_block_hash));
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


void BlockMessage::serialize(base::SerializationOArchive& oa, const lk::Block& block)
{
    oa.serialize(MessageType::BLOCK);
    oa.serialize(block);
}


BlockMessage BlockMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block = lk::Block::deserialize(ia);
    return { std::move(block) };
}


void BlockMessage::handle(const lk::MessageProcessor::Context& ctx)
{
    if (ctx.peer->getState() == lk::Peer::State::SYNCHRONISED) {
        ctx.core->tryAddBlock(std::move(_block));
    }
    else {
        lk::BlockDepth block_depth = _block.getDepth();
        ctx.peer->addSyncBlock(std::move(_block));

        if (block_depth == ctx.core->getTopBlock().getDepth() + 1) {
            ctx.peer->applySyncs();
        }
        else {
            ctx.peer->send(serializeMessage<GetBlockMessage>(ctx.peer->getSyncBlocks().front().getPrevBlockHash()));
        }
    }
}


BlockMessage::BlockMessage(lk::Block block)
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


BlockNotFoundMessage BlockNotFoundMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return { std::move(block_hash) };
}


void BlockNotFoundMessage::handle(const lk::MessageProcessor::Context&) {}


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


void GetInfoMessage::handle(const lk::MessageProcessor::Context& ctx)
{
    ctx.peer->send(serializeMessage<InfoMessage>(ctx.core->getTopBlock(), ctx.host->allConnectedPeersInfo()));
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


InfoMessage InfoMessage::deserialize(base::SerializationIArchive& ia)
{
    auto top_block_hash = ia.deserialize<base::Bytes>();
    auto available_peers = ia.deserialize<std::vector<net::Endpoint>>();
    return { std::move(top_block_hash), std::move(available_peers) };
}


void InfoMessage::handle(const lk::MessageProcessor::Context&) {}


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
                               const lk::Address& address)
{
    oa.serialize(MessageType::NEW_NODE);
    oa.serialize(new_node_endpoint);
    oa.serialize(address);
}


NewNodeMessage NewNodeMessage::deserialize(base::SerializationIArchive& ia)
{
    auto ep = net::Endpoint::deserialize(ia);
    auto address = ia.deserialize<lk::Address>();
    return { std::move(ep), std::move(address) };
}


void NewNodeMessage::handle(const lk::MessageProcessor::Context& ctx)
{
    ctx.host->checkOutPeer(_new_node_endpoint);
    ctx.host->broadcast(serializeMessage<NewNodeMessage>(_new_node_endpoint));
}


NewNodeMessage::NewNodeMessage(net::Endpoint&& new_node_endpoint, lk::Address&& address)
  : _new_node_endpoint{ std::move(new_node_endpoint) }
  , _address{ std::move(address) }
{}

//============================================
} // namespace


namespace lk
{

//============================================

MessageProcessor::MessageProcessor(MessageProcessor::Context context)
  : _ctx{ context }
{}

//============================================

namespace
{
template<typename F, typename... O>
bool runHandleImpl(MessageType mt, base::SerializationIArchive& ia, const lk::MessageProcessor::Context& ctx)
{
    if (F::getHandledMessageType() == mt) {
        auto message = F::deserialize(ia);
        message.handle(ctx);
        return true;
    }
    else {
        return runHandleImpl<O...>(mt, ia, ctx);
    }
}


template<>
bool runHandleImpl<void>(MessageType, base::SerializationIArchive&, const lk::MessageProcessor::Context&)
{
    return false;
}


template<typename... Args>
bool runHandle(MessageType mt, base::SerializationIArchive& ia, const lk::MessageProcessor::Context& ctx)
{
    return runHandleImpl<Args..., void>(mt, ia, ctx);
}
} // namespace


void MessageProcessor::process(const base::Bytes& raw_message)
{
    base::SerializationIArchive ia(raw_message);
    auto type = ia.deserialize<MessageType>();
    if (runHandle<HandshakeMessage,
                  PingMessage,
                  PongMessage,
                  TransactionMessage,
                  GetBlockMessage,
                  BlockMessage,
                  BlockNotFoundMessage,
                  GetInfoMessage,
                  InfoMessage,
                  NewNodeMessage>(type, ia, _ctx)) {
        LOG_DEBUG << "Processed  " << enumToString(type) << " message";
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "invalid message type");
    }
}

//============================================

// Network::Network()
//    _core.subscribeToBlockAddition(std::bind(&Network::onNewBlock, this, std::placeholders::_1));
//    _core.subscribeToNewPendingTransaction(std::bind(&Network::onNewPendingTransaction, this, std::placeholders::_1));
//}
//
//
//
//
// void Network::onNewBlock(const lk::Block& block)
//{
//    broadcast(serializeMessage<BlockMessage>(block));
//}
//
//
// void Network::onNewPendingTransaction(const lk::Transaction& tx)
//{
//    broadcast(serializeMessage<TransactionMessage>(tx));
//}


Protocol Protocol::peerConnected(MessageProcessor::Context context)
{
    Protocol ret{ std::move(context) };
    ret.startOnConnectedPeer();
    return ret;
}


Protocol Protocol::peerAccepted(MessageProcessor::Context context)
{
    Protocol ret{ std::move(context) };
    ret.startOnAcceptedPeer();
    return ret;
}


Protocol::Protocol(MessageProcessor::Context context)
  : _ctx{ context }
  , _processor{ context }
{
    doHandshake();
}


void Protocol::startOnAcceptedPeer()
{
    // TODO: _ctx.host->schedule(_peer.close); schedule disconnection on timeout
    // now does nothing, since we wait for connected peer to send us something (HANDSHAKE message)
}


void Protocol::startOnConnectedPeer()
{
    /*
     * we connected to a node, so now we are going to send handshake
     */
}


void Protocol::doHandshake() {}


void Protocol::onReceive(const base::Bytes& bytes)
{
    _processor.process(bytes);
}


void Protocol::onClose() {}


} // namespace core
