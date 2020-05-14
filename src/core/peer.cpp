#include "peer.hpp"

#include "base/utility.hpp"
#include "core/core.hpp"
#include "core/host.hpp"

namespace lk
{


/**
 * Connection protocol description:
 *  1) Connection to a given endpoint.
 *  2) Send CONNECT message, that contains IdentityInfo of host: public endpoint info and its LK address.
 *  3) Waiting for response. Valid responses are ACCEPTED and CANNOT_ACCEPT
 *      a) ACCEPTED response means that current peer was added to KademliaPeerPool of peers, that we are connecting to.
 *         Now we're ready for synchronisation.
 *      b) CANNOT_ACCEPT response means that current peer was rejected by some reason (this reason is provided in
 * message)
 *
 * Acception protocol description:
 *  1) Accepting connection.
 *  2) Expecting CONNECT message, receive it and retrieve IdentityInfo of that peer.
 *  3) Try to add peer to peer table.
 *      a) If adding is succeeded, then send ACCEPTED message. Now we're ready to synchronise.
 *      b) If adding is failed - send CANNOT_ACCEPT message. Shutdown the connection.
 *
 * Initial synchronisation protocol description.
 *  1) Initiated during handshake: if peers top-block if further than ours, then we request all blocks that we're lack
 * of. 2) All blocks are applied sequentially. 3) When initial top-block is applied, we say that host is synchronised
 * with given peer.
 *
 *
 *  Fix: do a synchronisation during runtime.
 */


Rating::Rating(std::int_fast32_t initial_value)
    : _value{initial_value}
{}


std::int_fast32_t Rating::getValue() const noexcept
{
    return _value;
}


Rating::operator bool() const noexcept
{
    return _value > 0;
}


Rating& Rating::nonExpectedMessage() noexcept
{
    _value -= 20;
    return *this;
}


Rating& Rating::invalidMessage() noexcept
{
    _value -= 30;
    return *this;
}


Rating& Rating::badBlock() noexcept
{
    _value -= 10;
    return *this;
}


Rating& Rating::differentGenesis() noexcept
{
    _value -= 2 * base::config::NET_INITIAL_PEER_RATING;
    return *this;
}


std::vector<Peer::IdentityInfo> PeerPoolBase::allPeersInfo() const
{
    std::vector<Peer::IdentityInfo> ret;
    forEachPeer([&ret](const Peer& peer) { ret.push_back(peer.getInfo()); });
    return ret;
}


Peer::IdentityInfo Peer::IdentityInfo::deserialize(base::SerializationIArchive& ia)
{
    auto endpoint = ia.deserialize<net::Endpoint>();
    auto address = ia.deserialize<lk::Address>();
    return Peer::IdentityInfo{ std::move(endpoint), std::move(address) };
}


void Peer::IdentityInfo::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(endpoint);
    oa.serialize(address);
}


void Peer::sendBlock(const base::Sha256& block_hash, const lk::Block& block)
{
    sendMessage(msg::Block{ block_hash, block });
}


void Peer::sendTransaction(const lk::Transaction& tx)
{
    sendMessage(msg::Transaction{ tx });
}


void Peer::requestLookup(const lk::Address& address, const std::uint8_t alpha)
{
    struct LookupData
    {
        LookupData(boost::asio::io_context& io_context, lk::Address address)
          : address{ address }
          , timer{ io_context }
        {
            timer.expires_after(std::chrono::seconds(base::config::NET_CONNECT_TIMEOUT));
        }

        bool was_responded{ false };
        lk::Address address;
        boost::asio::steady_timer timer;
    };

    auto data = std::make_shared<LookupData>(_host.getIoContext(), address);

    data->timer.async_wait([data](const auto& ec) { data->was_responded = true; });

    sendMessage(msg::Lookup{ address, alpha });
}


void Peer::requestBlock(const base::Sha256& block_hash)
{
    sendMessage(msg::GetBlock{block_hash});
}


std::shared_ptr<Peer> Peer::accepted(std::shared_ptr<net::Session> session, Context context)
{
    std::shared_ptr<Peer> ret(new Peer(std::move(session),
                                       false,
                                       context.host.getIoContext(),
                                       static_cast<lk::PeerPoolBase&>(context.host.getNonHandshakedPool()),
                                       static_cast<lk::KademliaPeerPoolBase&>(context.host.getHandshakedPool()),
                                       context.core,
                                       context.host));

    if (!context.host.getNonHandshakedPool().tryAddPeer(ret)) {
        RAISE_ERROR(base::Error, "cannot add peer");
    }

    ret->startSession();
    return ret;
}


std::shared_ptr<Peer> Peer::connected(std::shared_ptr<net::Session> session, Context context)
{
    std::shared_ptr<Peer> ret(new Peer(std::move(session),
                                       true,
                                       context.host.getIoContext(),
                                       static_cast<lk::PeerPoolBase&>(context.host.getNonHandshakedPool()),
                                       static_cast<lk::KademliaPeerPoolBase&>(context.host.getHandshakedPool()),
                                       context.core,
                                       context.host));

    if (!context.host.getNonHandshakedPool().tryAddPeer(ret)) {
        RAISE_ERROR(base::Error, "cannot add peer");
    }

    ret->startSession();
    return ret;
}


Peer::Peer(std::shared_ptr<net::Session> session,
           bool was_connected_to,
           boost::asio::io_context& io_context,
           lk::PeerPoolBase& non_handshaked_pool,
           lk::KademliaPeerPoolBase& handshaked_pool,
           lk::Core& core,
           lk::Host& host)
  : _session{ std::move(session) }
  , _was_connected_to{ was_connected_to }
  , _io_context{ io_context }
  , _address{ lk::Address::null() }
  , _non_handshaked_pool{ non_handshaked_pool }
  , _handshaked_pool{ handshaked_pool }
  , _core{ core }
  , _host{ host }
{
}


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


const lk::Address& Peer::getAddress() const noexcept
{
    return _address;
}


bool Peer::wasConnectedTo() const noexcept
{
    return _was_connected_to;
}


void Peer::startSession()
{
    if (_is_started) {
        return;
    }
    _is_started = true;
    _session->setHandler(std::make_unique<Handler>(weak_from_this()));
    _session->start();
    if (wasConnectedTo()) {
        sendMessage<msg::Connect>({ _core.getThisNodeAddress(), _host.getPublicPort(), _core.getTopBlockHash() });
    }
}


bool Peer::isClosed() const
{
    return !_session || _session->isClosed();
}


Peer::IdentityInfo Peer::getInfo() const
{
    return Peer::IdentityInfo{ getPublicEndpoint(), _address };
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
    LOG_DEBUG << this;
    LOG_DEBUG << shared_from_this().get();
    return _handshaked_pool.tryAddPeer(shared_from_this());
}


void Peer::detachFromPools()
{
    if (_is_attached_to_handshaked_pool) {
        _handshaked_pool.removePeer(this);
    }
    else {
        _non_handshaked_pool.removePeer(this);
    }
}

//===============================================

Peer::Synchronizer::Synchronizer(Peer& peer)
  : _peer{ peer }
{}


void Peer::Synchronizer::handleReceivedTopBlockHash(const base::Sha256& peers_top_block)
{
    // TODO: choose longest chain here
    const auto& ours_top_block_hash = _peer._core.getTopBlockHash();

    if (peers_top_block == ours_top_block_hash) {
        _peer.setState(lk::Peer::State::SYNCHRONISED);
        return; // nothing changes, because top blocks are equal
    }
    else {
        if (_peer._core.findBlock(peers_top_block)) {
            _peer.setState(lk::Peer::State::SYNCHRONISED);
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            base::SerializationOArchive oa;
            _peer.sendMessage(msg::GetBlock{ peers_top_block });
            _peer.setState(lk::Peer::State::REQUESTED_BLOCKS);
        }
    }
}


bool Peer::Synchronizer::handleReceivedBlock(const base::Sha256& hash, const Block& block)
{
    ASSERT(hash == base::Sha256::compute(base::toBytes(block)));

    if(_requested_block && *_requested_block == hash) {
        _requested_block.reset();
        _sync_blocks.push_back(block);
        const auto& next = block.getPrevBlockHash();
        if(next == base::Sha256::null()) {
            if(!_peer._rating.differentGenesis()) {
                // need to disconnect this peer since in has different genesis block, this is fatal
                _peer.endSession(msg::Close{});
            }
            return true;
        }
        else if(_peer._core.findBlock(next)) {
            // apply everything
        }
        else {
            requestBlock(block.getPrevBlockHash());
        }
        return true;
    }
    else if(_peer._core.tryAddBlock(block)) {
        return true;
    }
    else {
        return false;
    }
}


bool Peer::Synchronizer::isSynchronised() const
{
    return !_requested_block;
}


void Peer::Synchronizer::requestBlock(base::Sha256 block_hash)
{
    if(_requested_block) {
        RAISE_ERROR(base::RuntimeError, "block was already requested and not answered yet");
    }
    else {
        _peer.requestBlock(block_hash);
        _requested_block = std::move(block_hash);
    }
}

//===============================================

Peer::Handler::Handler(std::weak_ptr<Peer> peer)
  : _peer{ std::move(peer) }
{}


void Peer::Handler::onReceive(const base::Bytes& bytes)
{
    if (auto p = _peer.lock()) {
        p->process(bytes);
    }
}


void Peer::Handler::onClose()
{
    if (auto p = _peer.lock()) {
        p->detachFromPools();
    }
}

//===============================================

void Peer::process(const base::Bytes& received_data)
{
    base::SerializationIArchive ia(received_data);
    auto msg_type = ia.deserialize<msg::Type>();
    LOG_DEBUG << "Processing " << enumToString(msg_type);
    switch (msg_type) {
        case msg::Connect::TYPE_ID: {
            handle(ia.deserialize<msg::Connect>());
            break;
        }
        case msg::CannotAccept::TYPE_ID: {
            handle(ia.deserialize<msg::CannotAccept>());
            break;
        }
        case msg::Accepted::TYPE_ID: {
            handle(ia.deserialize<msg::Accepted>());
            break;
        }
        case msg::Ping::TYPE_ID: {
            handle(ia.deserialize<msg::Ping>());
            break;
        }
        case msg::Pong::TYPE_ID: {
            handle(ia.deserialize<msg::Pong>());
            break;
        }
        case msg::Lookup::TYPE_ID: {
            handle(ia.deserialize<msg::Lookup>());
            break;
        }
        case msg::LookupResponse::TYPE_ID: {
            handle(ia.deserialize<msg::LookupResponse>());
            break;
        }
        case msg::Transaction::TYPE_ID: {
            handle(ia.deserialize<msg::Transaction>());
            break;
        }
        case msg::GetBlock::TYPE_ID: {
            handle(ia.deserialize<msg::GetBlock>());
            break;
        }
        case msg::Block::TYPE_ID: {
            handle(ia.deserialize<msg::Block>());
            break;
        }
        case msg::BlockNotFound::TYPE_ID: {
            handle(ia.deserialize<msg::BlockNotFound>());
            break;
        }
        case msg::Close::TYPE_ID: {
            handle(ia.deserialize<msg::Close>());
            break;
        }
        default: {
            // this assertion checks if someone forgot to add case to switch
            ASSERT(static_cast<int>(msg::Type::DEBUG_MIN) >= static_cast<int>(msg_type) ||
                   static_cast<int>(msg::Type::DEBUG_MAX) <= static_cast<int>(msg_type));

            // invalid message type received - decrease rating
            _rating.invalidMessage();
        }
    }
    LOG_DEBUG << "Processed " << enumToString(msg_type);
}

//===============================================

void Peer::handle(lk::msg::Connect&& msg)
{
    if (msg.public_port) {
        auto public_ep = getEndpoint();
        public_ep.setPort(msg.public_port);
        setServerEndpoint(public_ep);
    }
    _address = msg.address;

    if (tryAddToPool()) {
        _non_handshaked_pool.removePeer(this);
        sendMessage(
          msg::Accepted{ _core.getThisNodeAddress(), getPublicEndpoint().getPort(), _core.getTopBlockHash() });

        requestLookup(getAddress(), base::config::NET_LOOKUP_ALPHA);
        _synchronizer.handleReceivedTopBlockHash(msg.top_block_hash);
    }
    else {
        endSession(
          msg::CannotAccept{ msg::CannotAccept::RefusionReason::BUCKET_IS_FULL, _host.allConnectedPeersInfo() });
    }
}


void Peer::handle(lk::msg::CannotAccept&& msg)
{
    for (const auto& peer : msg.peers_info) {
        _host.checkOutPeer(peer.endpoint, peer.address);
    }
}


void Peer::handle(lk::msg::Accepted&& msg)
{
    if (msg.public_port) {
        auto public_ep = getEndpoint();
        public_ep.setPort(msg.public_port);
        setServerEndpoint(public_ep);
    }
    _address = msg.address;

    sendMessage(msg::Lookup{ getAddress(), base::config::NET_LOOKUP_ALPHA });

    if (tryAddToPool()) {
        _non_handshaked_pool.removePeer(this);
        _synchronizer.handleReceivedTopBlockHash(msg.top_block_hash);
    }
    else {
        endSession(
          msg::CannotAccept{ msg::CannotAccept::RefusionReason::BUCKET_IS_FULL, _host.allConnectedPeersInfo() });
    }
}


void Peer::handle(lk::msg::Ping&&) {}


void Peer::handle(lk::msg::Pong&&) {}


void Peer::handle(lk::msg::Lookup&& msg)
{
    auto reply = _handshaked_pool.lookup(msg.address, msg.selection_size);
    sendMessage(msg::LookupResponse{ msg.address, std::move(reply) });
}


void Peer::handle(lk::msg::LookupResponse&& msg)
{
    // either we continue to ask for closest nodes or just connect to them
    // TODO: a peer table, where we ask for LOOKUP, and collect their responds + change the very beginning of
    // communication: now it is not necessary to do a HANDSHAKE if we just want to ask for LOOKUP

    if constexpr(base::config::IS_DEBUG) {
        std::ostringstream ss;
        ss << "Lookup response peer entries:\n";
        for(const auto& pi : msg.peers_info) {
            ss << '\t' << pi.endpoint << ' ' << pi.address << '\n';
        }
        LOG_DEBUG << ss.str();
    }

    for (const auto& pi : msg.peers_info) {
        _host.checkOutPeer(pi.endpoint, pi.address);
    }
}


void Peer::handle(lk::msg::Transaction&& msg)
{
    _core.addPendingTransaction(msg.tx);
}


void Peer::handle(lk::msg::GetBlock&& msg)
{
    LOG_DEBUG << "Received GET_BLOCK on " << msg.block_hash;
    if (auto block = _core.findBlock(msg.block_hash)) {
        sendMessage(msg::Block{ msg.block_hash, *block });
    }
    else {
        sendMessage(msg::BlockNotFound{ msg.block_hash });
    }
}


void Peer::handle(lk::msg::Block&& msg)
{
    if(msg.block_hash != base::Sha256::compute(base::toBytes(msg.block))) {
        _rating.invalidMessage();
        return;
    }

    if (!_synchronizer.handleReceivedBlock(msg.block_hash, msg.block)) {
        // strange block received, decrease rating
        _rating.badBlock();
    }
}


void Peer::handle(lk::msg::BlockNotFound&& msg)
{
    LOG_DEBUG << "Block not found " << msg.block_hash;
}


void Peer::handle(lk::msg::Close&& msg) {
    detachFromPools();
    _session->close();
}

}

//===============================================
