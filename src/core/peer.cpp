#include "peer.hpp"

#include "base/utility.hpp"
#include "core/core.hpp"
#include "core/host.hpp"

namespace lk
{

//========================================

class CannotAcceptMessage
{
  public:
    DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(RefusionReason, std::uint8_t, (NOT_AVAILABLE)(BUCKET_IS_FULL)(BAD_RATING))

    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, RefusionReason why_not_accepted);
    static CannotAcceptMessage deserialize(base::SerializationIArchive& ia);
    void handle(const Peer::Context& ctx, Peer& peer);

  private:
    RefusionReason _why_not_accepted;
    std::vector<lk::PeerBase::Info> _peers_info;

    CannotAcceptMessage(RefusionReason why_not_accepted, std::vector<lk::PeerBase::Info> peers_info);
};

//========================================

class AcceptedMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const lk::Block& top_block,
                          const lk::Address& address,
                          std::uint16_t public_port,
                          const std::vector<lk::Peer::Info>& known_peers);
    static AcceptedMessage deserialize(base::SerializationIArchive& ia);
    void handle(const Peer::Context& context, Peer& peer);

  private:
    lk::Block _theirs_top_block;
    lk::Address _address;
    std::uint16_t
      _public_port; // zero public port states that peer didn't provide information about his public endpoint
    std::vector<lk::Peer::Info> _known_peers;

    AcceptedMessage(lk::Block&& top_block,
                    lk::Address address,
                    std::uint16_t public_port,
                    std::vector<lk::Peer::Info>&& known_peers);
};

//========================================

class AcceptedResponseMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const lk::Block& top_block,
                          const lk::Address& address,
                          std::uint16_t public_port,
                          const std::vector<lk::Peer::Info>& known_peers);
    static AcceptedResponseMessage deserialize(base::SerializationIArchive& ia);
    void handle(const Peer::Context& context, Peer& peer);

  private:
    lk::Block _theirs_top_block;
    lk::Address _address;
    std::uint16_t
      _public_port; // zero public port states that peer didn't provide information about his public endpoint
    std::vector<lk::Peer::Info> _known_peers;

    AcceptedResponseMessage(lk::Block&& top_block,
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
    void handle(const Peer::Context& ctx, Peer& peer);

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
    void handle(const Peer::Context& ctx, Peer& peer);

  private:
    PongMessage() = default;
};

//============================================

class LookupMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const lk::Address& address, std::uint8_t selection_size);
    static LookupMessage deserialize(base::SerializationIArchive& ia);
    void handle(const Peer::Context& ctx, Peer& peer);

  private:
    lk::Address _address;
    std::uint8_t _selection_size;
    LookupMessage(lk::Address address, std::uint8_t selection_size);
};

//============================================

class LookupResponseMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const lk::Address& address, const std::vector<lk::PeerBase::Info>& peers_info);
    static LookupResponseMessage deserialize(base::SerializationIArchive& ia);
    void handle(const Peer::Context& ctx, Peer& peer);

  private:
    lk::Address _address;
    std::vector<lk::PeerBase::Info> _peers_info;
    LookupResponseMessage(lk::Address address, std::vector<lk::PeerBase::Info> peers);
};

//============================================

class TransactionMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const lk::Transaction& tx);
    static TransactionMessage deserialize(base::SerializationIArchive& ia);
    void handle(const Peer::Context& ctx, Peer& peer);

  private:
    lk::Transaction _tx;

    TransactionMessage(const lk::Transaction& tx);
};

//============================================

class GetBlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    static GetBlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(const Peer::Context& ctx, Peer& peer);

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
    void handle(const Peer::Context& ctx, Peer& peer);

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
    void handle(const Peer::Context& ctx, Peer& peer);

  private:
    base::Sha256 _block_hash;

    BlockNotFoundMessage(base::Sha256 block_hash);
};

//============================================

class CloseMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static CloseMessage deserialize(base::SerializationIArchive& ia);
    void handle(const Peer::Context& ctx, Peer& peer);

  private:
    CloseMessage();
};

//============================================

Peer::Info Peer::Info::deserialize(base::SerializationIArchive& ia)
{
    auto endpoint = ia.deserialize<net::Endpoint>();
    auto address = ia.deserialize<lk::Address>();
    Peer::Info ret{ std::move(endpoint), std::move(address) };
    return ret;
}


void Peer::Info::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(endpoint);
    oa.serialize(address);
}

//================================

/*
 * Some functions, that are implemented and later used inside templates are shown as not used.
 * So, warnings for unused functions are just disabled for this file.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

namespace
{

template<typename M, typename... Args>
base::Bytes prepareMessage(Args&&... args)
{
    LOG_TRACE << "Serializing " << enumToString(M::getHandledMessageType());
    base::SerializationOArchive oa;
    oa.serialize(M::getHandledMessageType());
    (oa.serialize(std::forward<Args>(args)), ...);
    return std::move(oa).getBytes();
}


class Dummy
{};

template<typename C, typename F, typename... O>
bool runHandleImpl([[maybe_unused]] lk::MessageType mt, base::SerializationIArchive& ia, const C& ctx, Peer& peer)
{
    if constexpr (std::is_same<F, Dummy>::value) {
        return false;
    }
    else {
        if (F::getHandledMessageType() == mt) {
            auto message = F::deserialize(ia);
            message.handle(ctx, peer);
            return true;
        }
        else {
            return runHandleImpl<C, O...>(mt, ia, ctx, peer);
        }
    }
}


template<typename C, typename... Args>
bool runHandle(lk::MessageType mt, base::SerializationIArchive& ia, const C& ctx, Peer& peer)
{
    if (runHandleImpl<C, Args..., Dummy>(mt, ia, ctx, peer)) {
        return true;
    }
    else {
        return false;
    }
}


template<typename C>
class MessageProcessor
{
  public:
    //===============
    explicit MessageProcessor(const C& context, Peer& peer)
      : _ctx{ context }
      , _peer{ peer }
    {}

    /*
     * Decode and act according to received data.
     * Throws if invalid message, or there is an error during handling.
     */
    void process(const base::Bytes& raw_message)
    {
        using namespace lk;

        base::SerializationIArchive ia(raw_message);
        auto type = ia.deserialize<lk::MessageType>();

        if (runHandle<Peer::Context,
                      AcceptedMessage,
                      AcceptedResponseMessage,
                      PingMessage,
                      PongMessage,
                      TransactionMessage,
                      GetBlockMessage,
                      BlockMessage,
                      BlockNotFoundMessage,
                      CloseMessage>(type, ia, _ctx, _peer)) {
            LOG_DEBUG << "Processed " << enumToString(type) << " message";
        }
        else {
            RAISE_ERROR(base::InvalidArgument, "invalid message type");
        }
    }

  private:
    const C& _ctx;
    Peer& _peer;
};


std::vector<lk::PeerBase::Info> allPeersInfoExcept(lk::PeerPoolBase& host, const lk::Address& address)
{
    auto ret = host.allPeersInfo();
    ret.erase(std::find_if(ret.begin(), ret.end(), [address](const auto& cand) { return cand.address == address; }));
    return ret;
}


} // namespace

//============================================

constexpr lk::MessageType CannotAcceptMessage::getHandledMessageType()
{
    return lk::MessageType::CANNOT_ACCEPT;
}


void CannotAcceptMessage::serialize(base::SerializationOArchive& oa,
                                    CannotAcceptMessage::RefusionReason why_not_accepted)
{
    oa.serialize(getHandledMessageType());
    oa.serialize(why_not_accepted);
}


CannotAcceptMessage CannotAcceptMessage::deserialize(base::SerializationIArchive& ia)
{
    auto why_not_accepted = ia.deserialize<RefusionReason>();
    auto peers_info = ia.deserialize<std::vector<lk::PeerBase::Info>>();
    return CannotAcceptMessage(why_not_accepted, std::move(peers_info));
}


void CannotAcceptMessage::handle(const Peer::Context& ctx, lk::Peer& peer)
{
    ctx.pool.removePeer(&peer);

    for (const auto& peer : _peers_info) {
        ctx.host.checkOutPeer(peer.endpoint, [](std::shared_ptr<Peer> peer) {
            peer->startSession();
        });
    }
}


CannotAcceptMessage::CannotAcceptMessage(CannotAcceptMessage::RefusionReason why_not_accepted,
                                         std::vector<lk::PeerBase::Info> peers_info)
  : _why_not_accepted{ why_not_accepted }
  , _peers_info{ std::move(peers_info) }
{}

//============================================

constexpr lk::MessageType AcceptedMessage::getHandledMessageType()
{
    return lk::MessageType::ACCEPTED;
}


void AcceptedMessage::serialize(base::SerializationOArchive& oa,
                                const lk::Block& block,
                                const lk::Address& address,
                                std::uint16_t public_port,
                                const std::vector<lk::Peer::Info>& known_peers)
{
    oa.serialize(getHandledMessageType());
    oa.serialize(block);
    oa.serialize(address);
    oa.serialize(public_port);
    oa.serialize(known_peers);
}


AcceptedMessage AcceptedMessage::deserialize(base::SerializationIArchive& ia)
{
    auto top_block = ia.deserialize<lk::Block>();
    auto address = ia.deserialize<lk::Address>();
    auto public_port = ia.deserialize<std::uint16_t>();
    auto known_peers = ia.deserialize<std::vector<lk::Peer::Info>>();
    return AcceptedMessage(std::move(top_block), std::move(address), public_port, std::move(known_peers));
}


void AcceptedMessage::handle(const Peer::Context& ctx, lk::Peer& peer)
{
    const auto& ours_top_block = ctx.core.getTopBlock();

    peer.send(prepareMessage<AcceptedResponseMessage>(ctx.core.getTopBlock(),
                                                      ctx.core.getThisNodeAddress(),
                                                      peer.getPublicEndpoint().getPort(),
                                                      allPeersInfoExcept(ctx.host.getPool(), peer.getAddress())), {});

    if (_public_port) {
        auto public_ep = peer.getEndpoint();
        public_ep.setPort(_public_port);
        peer.setServerEndpoint(public_ep);
    }

    for (const auto& peer_info : _known_peers) {
        ctx.host.checkOutPeer(peer_info.endpoint, [](std::shared_ptr<Peer> peer) {
            peer->startSession();
        });
    }

    if (_theirs_top_block == ours_top_block) {
        peer.setState(lk::Peer::State::SYNCHRONISED);
        return; // nothing changes, because top blocks are equal
    }
    else {
        if (ours_top_block.getDepth() > _theirs_top_block.getDepth()) {
            peer.setState(lk::Peer::State::SYNCHRONISED);
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            if (ctx.core.getTopBlock().getDepth() + 1 == _theirs_top_block.getDepth()) {
                ctx.core.tryAddBlock(_theirs_top_block);
                peer.setState(lk::Peer::State::SYNCHRONISED);
            }
            else {
                base::SerializationOArchive oa;
                GetBlockMessage::serialize(oa, _theirs_top_block.getPrevBlockHash());
                peer.send(std::move(oa).getBytes(), {});
                peer.setState(lk::Peer::State::REQUESTED_BLOCKS);
                peer.addSyncBlock(std::move(_theirs_top_block));
            }
        }
    }
}


AcceptedMessage::AcceptedMessage(lk::Block&& top_block,
                                 lk::Address address,
                                 std::uint16_t public_port,
                                 std::vector<lk::Peer::Info>&& known_peers)
  : _theirs_top_block{ std::move(top_block) }
  , _address{ std::move(address) }
  , _public_port{ public_port }
  , _known_peers{ std::move(known_peers) }
{}

//============================================

constexpr lk::MessageType AcceptedResponseMessage::getHandledMessageType()
{
    return lk::MessageType::ACCEPTED_RESPONSE;
}


void AcceptedResponseMessage::serialize(base::SerializationOArchive& oa,
                                        const lk::Block& block,
                                        const lk::Address& address,
                                        std::uint16_t public_port,
                                        const std::vector<lk::Peer::Info>& known_peers)
{
    oa.serialize(getHandledMessageType());
    oa.serialize(block);
    oa.serialize(address);
    oa.serialize(public_port);
    oa.serialize(known_peers);
}


AcceptedResponseMessage AcceptedResponseMessage::deserialize(base::SerializationIArchive& ia)
{
    auto top_block = ia.deserialize<lk::Block>();
    auto address = ia.deserialize<lk::Address>();
    auto public_port = ia.deserialize<std::uint16_t>();
    auto known_peers = ia.deserialize<std::vector<lk::Peer::Info>>();
    return AcceptedResponseMessage(std::move(top_block), std::move(address), public_port, std::move(known_peers));
}


void AcceptedResponseMessage::handle(const Peer::Context& ctx, lk::Peer& peer)
{
    const auto& ours_top_block = ctx.core.getTopBlock();

    if (_public_port) {
        auto public_ep = peer.getEndpoint();
        public_ep.setPort(_public_port);
        peer.setServerEndpoint(public_ep);
    }

    for (const auto& peer_info : _known_peers) {
        ctx.host.checkOutPeer(peer_info.endpoint, [](std::shared_ptr<Peer> peer) {
            peer->startSession();
        });
    }

    if (_theirs_top_block == ours_top_block) {
        peer.setState(lk::Peer::State::SYNCHRONISED);
        return; // nothing changes, because top blocks are equal
    }
    else {
        if (ours_top_block.getDepth() > _theirs_top_block.getDepth()) {
            peer.setState(lk::Peer::State::SYNCHRONISED);
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            if (ctx.core.getTopBlock().getDepth() + 1 == _theirs_top_block.getDepth()) {
                ctx.core.tryAddBlock(_theirs_top_block);
                peer.setState(lk::Peer::State::SYNCHRONISED);
            }
            else {
                base::SerializationOArchive oa;
                GetBlockMessage::serialize(oa, _theirs_top_block.getPrevBlockHash());
                peer.send(std::move(oa).getBytes(), {});
                peer.setState(lk::Peer::State::REQUESTED_BLOCKS);
                peer.addSyncBlock(std::move(_theirs_top_block));
            }
        }
    }
}


AcceptedResponseMessage::AcceptedResponseMessage(lk::Block&& top_block,
                                                 lk::Address address,
                                                 std::uint16_t public_port,
                                                 std::vector<lk::Peer::Info>&& known_peers)
  : _theirs_top_block{ std::move(top_block) }
  , _address{ std::move(address) }
  , _public_port{ public_port }
  , _known_peers{ std::move(known_peers) }
{}

//============================================

constexpr lk::MessageType PingMessage::getHandledMessageType()
{
    return lk::MessageType::PING;
}


void PingMessage::serialize(base::SerializationOArchive& oa)
{
    oa.serialize(lk::MessageType::PING);
}


PingMessage PingMessage::deserialize(base::SerializationIArchive&)
{
    return {};
}


void PingMessage::handle(const Peer::Context&, lk::Peer& peer) {}


//============================================

constexpr lk::MessageType PongMessage::getHandledMessageType()
{
    return lk::MessageType::PONG;
}


void PongMessage::serialize(base::SerializationOArchive& oa)
{
    oa.serialize(lk::MessageType::PONG);
}


PongMessage PongMessage::deserialize(base::SerializationIArchive&)
{
    return {};
}


void PongMessage::handle(const Peer::Context&, Peer&) {}

//============================================

constexpr lk::MessageType LookupMessage::getHandledMessageType()
{
    return lk::MessageType::LOOKUP;
}


void LookupMessage::serialize(base::SerializationOArchive& oa, const lk::Address& address, std::uint8_t selection_size)
{
    oa.serialize(lk::MessageType::LOOKUP);
    oa.serialize(address);
    oa.serialize(selection_size);
}


LookupMessage LookupMessage::deserialize(base::SerializationIArchive& ia) {
    auto address = ia.deserialize<lk::Address>();
    auto selection_size = ia.deserialize<std::uint8_t>();
    return LookupMessage{std::move(address), selection_size};
}


void LookupMessage::handle(const Peer::Context& ctx, lk::Peer& peer)
{
    auto reply = ctx.pool.lookup(_address, _selection_size);
    peer.send(prepareMessage<LookupResponseMessage>(reply), {});
}


LookupMessage::LookupMessage(lk::Address address, std::uint8_t selection_size)
  : _address{ std::move(address) }
  , _selection_size{ selection_size }
{}

//============================================

constexpr lk::MessageType LookupResponseMessage::getHandledMessageType()
{
    return lk::MessageType::LOOKUP;
}


void LookupResponseMessage::serialize(base::SerializationOArchive& oa,
                                      const lk::Address& address,
                                      const std::vector<lk::PeerBase::Info>& peers_info)
{
    oa.serialize(lk::MessageType::LOOKUP_RESPONSE);
    oa.serialize(address);
    oa.serialize(peers_info);
}


LookupResponseMessage LookupResponseMessage::deserialize(base::SerializationIArchive& ia)
{
    auto address = ia.deserialize<lk::Address>();
    auto peers_info = ia.deserialize<std::vector<lk::PeerBase::Info>>();
    return LookupResponseMessage(std::move(address), std::move(peers_info));
}


void LookupResponseMessage::handle(const Peer::Context& ctx, lk::Peer& peer)
{
    // either we continue to ask for closest nodes or just connect to them
    // TODO: a peer table, where we ask for LOOKUP, and collect their responds + change the very beginning of
    // communication: now it is not necessary to do a HANDSHAKE if we just want to ask for LOOKUP

    if(auto it = peer._lookup_callbacks.find(_address); it != peer._lookup_callbacks.end()) {
        auto callback = std::move(it->second);
        peer._lookup_callbacks.erase(it);
        callback(_peers_info);
    }
}


LookupResponseMessage::LookupResponseMessage(lk::Address address, std::vector<lk::PeerBase::Info> peers_info)
  : _address{std::move(address)}, _peers_info{ std::move(peers_info) }
{}

//============================================

constexpr lk::MessageType TransactionMessage::getHandledMessageType()
{
    return lk::MessageType::TRANSACTION;
}


void TransactionMessage::serialize(base::SerializationOArchive& oa, const lk::Transaction& tx)
{
    oa.serialize(lk::MessageType::TRANSACTION);
    oa.serialize(tx);
}


TransactionMessage TransactionMessage::deserialize(base::SerializationIArchive& ia)
{
    auto tx = lk::Transaction::deserialize(ia);
    return { std::move(tx) };
}


void TransactionMessage::handle(const Peer::Context& ctx, lk::Peer& peer)
{
    ctx.core.addPendingTransaction(_tx);
}


TransactionMessage::TransactionMessage(const lk::Transaction& tx)
  : _tx{ std::move(tx) }
{}

//============================================

constexpr lk::MessageType GetBlockMessage::getHandledMessageType()
{
    return lk::MessageType::GET_BLOCK;
}


void GetBlockMessage::serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash)
{
    oa.serialize(lk::MessageType::GET_BLOCK);
    oa.serialize(block_hash);
}


GetBlockMessage GetBlockMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return { std::move(block_hash) };
}


void GetBlockMessage::handle(const Peer::Context& ctx, lk::Peer& peer)
{
    LOG_DEBUG << "Received GET_BLOCK on " << _block_hash;
    auto block = ctx.core.findBlock(_block_hash);
    if (block) {
        peer.send(prepareMessage<BlockMessage>(*block), {});
    }
    else {
        peer.send(prepareMessage<BlockNotFoundMessage>(_block_hash), {});
    }
}


GetBlockMessage::GetBlockMessage(base::Sha256 block_hash)
  : _block_hash{ std::move(block_hash) }
{}

//============================================

constexpr lk::MessageType BlockMessage::getHandledMessageType()
{
    return lk::MessageType::BLOCK;
}


void BlockMessage::serialize(base::SerializationOArchive& oa, const lk::Block& block)
{
    oa.serialize(lk::MessageType::BLOCK);
    oa.serialize(block);
}


BlockMessage BlockMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block = lk::Block::deserialize(ia);
    return { std::move(block) };
}


void BlockMessage::handle(const Peer::Context& ctx, lk::Peer& peer)
{
    if (peer.getState() == lk::Peer::State::SYNCHRONISED) {
        // we're synchronised already

        if (ctx.core.tryAddBlock(_block)) {
            // block added, all is OK
        }
        else {
            // in this case we are missing some blocks
        }
    }
    else {
        // we are in synchronization process
        lk::BlockDepth block_depth = _block.getDepth();
        peer.addSyncBlock(std::move(_block));

        if (block_depth == ctx.core.getTopBlock().getDepth() + 1) {
            peer.applySyncs();
        }
        else {
            peer.send(prepareMessage<GetBlockMessage>(peer.getSyncBlocks().front().getPrevBlockHash()), {});
        }
    }
}


BlockMessage::BlockMessage(lk::Block block)
  : _block{ std::move(block) }
{}

//============================================

constexpr lk::MessageType BlockNotFoundMessage::getHandledMessageType()
{
    return lk::MessageType::BLOCK_NOT_FOUND;
}


void BlockNotFoundMessage::serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash)
{
    oa.serialize(lk::MessageType::BLOCK_NOT_FOUND);
    oa.serialize(block_hash);
}


BlockNotFoundMessage BlockNotFoundMessage::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return { std::move(block_hash) };
}


void BlockNotFoundMessage::handle(const Peer::Context&, lk::Peer& peer)
{
    LOG_DEBUG << "Block not found " << _block_hash;
}


BlockNotFoundMessage::BlockNotFoundMessage(base::Sha256 block_hash)
  : _block_hash{ std::move(block_hash) }
{}

//============================================

constexpr lk::MessageType CloseMessage::getHandledMessageType()
{
    return lk::MessageType::CLOSE;
}


void CloseMessage::serialize(base::SerializationOArchive& oa)
{
    oa.serialize(getHandledMessageType());
}


CloseMessage CloseMessage::deserialize(base::SerializationIArchive&)
{
    return {};
}


void CloseMessage::handle(const Peer::Context&, Peer&) {}


CloseMessage::CloseMessage() {}

//============================================


void Peer::sendBlock(const lk::Block& block)
{
    send(prepareMessage<BlockMessage>(block), {});
}


void Peer::sendTransaction(const lk::Transaction& tx)
{
    send(prepareMessage<TransactionMessage>(tx), {});
}


void Peer::sendSessionEnd(std::function<void()> on_send)
{
    send(prepareMessage<CloseMessage>(), std::move(on_send));
}


void Peer::lookup(const lk::Address& address,
                  const std::size_t alpha,
                  std::function<void(std::vector<PeerBase::Info>)> callback)
{
    struct LookupData
    {
        LookupData(boost::asio::io_context& io_context, lk::Address address)
            : address{address}, timer{io_context}
        {
            timer.expires_after(std::chrono::seconds(base::config::NET_CONNECT_TIMEOUT));
        }

        bool was_responded{false};
        lk::Address address;
        boost::asio::steady_timer timer;
    };

    auto data = std::make_shared<LookupData>(_host.getIoContext(), address);

    data->timer.async_wait([this, data](const auto& ec) {
        data->was_responded = true;
        _lookup_callbacks.erase(_lookup_callbacks.find(data->address));
    });

    _lookup_callbacks.insert({address, [data, callback = std::move(callback)](auto peers_info) {
        callback(peers_info);
    }});

    send(prepareMessage<LookupMessage>(address, alpha), {});
}

#pragma GCC diagnostic pop

std::shared_ptr<Peer> Peer::accepted(std::unique_ptr<net::Session> session, lk::Host& host, lk::Core& core)
{
    std::shared_ptr<Peer> ret(new Peer(std::move(session), false, host.getIoContext(), host.getPool(), core, host));
    ret->startSession();
    return ret;
}


std::shared_ptr<Peer> Peer::connected(std::unique_ptr<net::Session> session, lk::Host& host, lk::Core& core)
{
    std::shared_ptr<Peer> ret(new Peer(std::move(session), true, host.getIoContext(), host.getPool(), core, host));
    ret->startSession();
    return ret;
}


Peer::Peer(std::unique_ptr<net::Session> session, bool is_connected, boost::asio::io_context& io_context, lk::PeerPoolBase& pool, lk::Core& core, lk::Host& host)
  : _session{ std::move(session) }
  , _io_context{ io_context }
  , _address{ lk::Address::null() }
  , _was_connected_to{ is_connected }
  , _pool{ pool }
  , _core{ core }
  , _host{ host }
{}


net::Endpoint Peer::getEndpoint() const
{
    return _session->getEndpoint();
}


net::Endpoint Peer::getPublicEndpoint() const
{
    return *_endpoint_for_incoming_connections;
}


void Peer::setServerEndpoint(net::Endpoint endpoint)
{
    _endpoint_for_incoming_connections = std::move(endpoint);
}


base::Time Peer::getLastSeen() const
{
    return _session->getLastSeen();
}


void Peer::addSyncBlock(lk::Block block)
{
    _sync_blocks.push_front(std::move(block));
}


void Peer::applySyncs()
{
    for (auto&& sync : _sync_blocks) {
        _core.tryAddBlock(sync);
    }
    _sync_blocks.clear();
}


const std::forward_list<lk::Block>& Peer::getSyncBlocks() const noexcept
{
    return _sync_blocks;
}


void Peer::send(const base::Bytes& data, net::Connection::SendHandler on_send = {})
{
    _session->send(data, std::move(on_send));
}


void Peer::send(base::Bytes&& data, net::Connection::SendHandler on_send = {})
{
    _session->send(std::move(data), std::move(on_send));
}


const lk::Address& Peer::getAddress() const noexcept
{
    return _address;
}


void Peer::setAddress(lk::Address address)
{
    _address = std::move(address);
}


bool Peer::wasConnectedTo() const noexcept
{
    return _was_connected_to;
}


void Peer::startSession()
{
    if(_session->isActive()) {
        return;
    }

    _session->start();
    if (wasConnectedTo()) {
        /*
         * we connected to a node, so now we are waiting for:
         * 1) success response ---> handshake message
         * 2) failure response ---> cannot accept message
         * 3) for timeout
         */
    }
    else {
        // TODO: _ctx.pool->schedule(_peer.close); schedule disconnection on timeout
        // now does nothing, since we wait for connected peer to send us something (HANDSHAKE message)
        if (tryAddToPool()) {
            send(prepareMessage<AcceptedMessage>(_core.getTopBlock(),
                                                 _core.getThisNodeAddress(),
                                                 getPublicEndpoint().getPort(),
                                                 allPeersInfoExcept(_pool, getAddress())));
        }
        else {
            send(prepareMessage<CannotAcceptMessage>(CannotAcceptMessage::RefusionReason::BUCKET_IS_FULL,
                                                     _pool.allPeersInfo()));
            // and close peer properly
        }
    }
}


bool Peer::isClosed() const
{
    return !_session || _session->isClosed();
}


Peer::Info Peer::getInfo() const
{
    return Peer::Info{ _session->getEndpoint(), _address };
}


void Peer::setState(Peer::State state)
{
    _state = state;
}


Peer::State Peer::getState() const noexcept
{
    return _state;
}


bool Peer::tryAddToPool()
{
    return _pool.tryAddPeer(shared_from_this());
}

}