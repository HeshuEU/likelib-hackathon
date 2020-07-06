#include "peer.hpp"

#include "base/utility.hpp"
#include "core/core.hpp"
#include "core/host.hpp"

namespace lk
{


#ifdef CONFIG_IS_DEBUG
#define PEER_LOG LOG_DEBUG << "Peer " << this << " | "
#else
#define PEER_LOG LOG_DEBUG
#endif

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

//===============================================

void Peer::sendBlock(const ImmutableBlock& block)
{
    _requests.send(msg::Block{ block.getHash(), block });
}


void Peer::sendNewBlock(const ImmutableBlock& block)
{
    _requests.send(msg::NewBlock{ block.getHash(), block });
}


void Peer::sendTransaction(const Transaction& tx)
{
    _requests.send(msg::Transaction{ tx });
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

    _requests.send(msg::Lookup{ address, alpha });
}


void Peer::requestBlock(const base::Sha256& block_hash)
{
    PEER_LOG << "requesting block " << block_hash;
    _requests.send(msg::GetBlock{ block_hash });
}


std::shared_ptr<Peer> Peer::accepted(std::shared_ptr<net::Session> session, Rating rating, Context context)
{
    std::shared_ptr<Peer> peer{ new Peer(std::move(session),
                                         false,
                                         context.host.getIoContext(),
                                         std::move(rating),
                                         static_cast<lk::PeerPoolBase&>(context.host.getNonHandshakedPool()),
                                         static_cast<lk::KademliaPeerPoolBase&>(context.host.getHandshakedPool()),
                                         context.core,
                                         context.host) };

    auto ret = peer->shared_from_this();

    if (!rating) {
        peer->endSession(msg::CannotAccept{});
        return {};
    }

    if (!context.host.getNonHandshakedPool().tryAddPeer(ret)) {
        RAISE_ERROR(base::Error, "cannot add peer");
    }

    ret->startSession();
    return ret;
}


std::shared_ptr<Peer> Peer::connected(std::shared_ptr<net::Session> session, Rating rating, Context context)
{
    std::shared_ptr<Peer> peer{ new Peer(std::move(session),
                                         true,
                                         context.host.getIoContext(),
                                         std::move(rating),
                                         static_cast<lk::PeerPoolBase&>(context.host.getNonHandshakedPool()),
                                         static_cast<lk::KademliaPeerPoolBase&>(context.host.getHandshakedPool()),
                                         context.core,
                                         context.host) };
    peer->setServerEndpoint(peer->getEndpoint());

    auto ret = peer->shared_from_this();

    if (!rating) {
        peer->endSession(msg::CannotAccept{});
        return {};
    }

    if (!context.host.getNonHandshakedPool().tryAddPeer(ret)) {
        RAISE_ERROR(base::Error, "cannot add peer");
    }

    ret->startSession();
    return ret;
}


Peer::Peer(std::shared_ptr<net::Session> session,
           bool was_connected_to,
           boost::asio::io_context& io_context,
           Rating rating,
           lk::PeerPoolBase& non_handshaked_pool,
           lk::KademliaPeerPoolBase& handshaked_pool,
           lk::Core& core,
           lk::Host& host)
  : _session{ std::move(session) }
  , _was_connected_to{ was_connected_to }
  , _io_context{ io_context }
  , _address{ lk::Address::null() }
  , _rating{ std::move(rating) }
  , _non_handshaked_pool{ non_handshaked_pool }
  , _handshaked_pool{ handshaked_pool }
  , _core{ core }
  , _host{ host }
  , _requests{ std::weak_ptr{ _session }, _io_context }
{
    PEER_LOG << "Peer has endpoint " << _session->getEndpoint();
}


Peer::~Peer()
{
    PEER_LOG << " is destroyed";
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
    PEER_LOG << "setting public endpoint to " << endpoint;
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

    _requests.setDefaultCallback([peer_holder = weak_from_this()](base::SerializationIArchive&& ia) {
        if (auto peer = peer_holder.lock()) {
            peer->process(std::move(ia));
        }
    });

    _requests.setCloseCallback([peer_holder = weak_from_this()] {
        if (auto peer = peer_holder.lock()) {
            peer->detachFromPools();
        }
    });

    _session->start();
    if (wasConnectedTo()) {
        _requests.send(msg::Connect{ _core.getThisNodeAddress(), _host.getPublicPort(), _core.getTopBlockHash() });
    }
}


bool Peer::isSessionClosed() const
{
    return !_session || _session->isClosed();
}


msg::NodeIdentityInfo Peer::getInfo() const
{
    return msg::NodeIdentityInfo{ getPublicEndpoint(), _address };
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
    return _handshaked_pool.tryAddPeer(shared_from_this());
}


void Peer::detachFromPools()
{
    _handshaked_pool.tryRemovePeer(this);
    _non_handshaked_pool.tryRemovePeer(this);
}

//===============================================

std::vector<msg::NodeIdentityInfo> PeerPoolBase::allPeersInfo() const
{
    std::vector<msg::NodeIdentityInfo> ret;
    forEachPeer([&ret](const Peer& peer) { ret.push_back(peer.getInfo()); });
    return ret;
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
            LOG_DEBUG << "Peer" << &_peer << " requesting " << peers_top_block << " block";
            requestBlock(peers_top_block);
            _peer.setState(lk::Peer::State::REQUESTED_BLOCKS);
        }
    }
}


bool Peer::Synchronizer::handleReceivedBlock(const base::Sha256& hash, const ImmutableBlock& block)
{
    ASSERT(hash == block.getHash()); // not only assert, but check if the message is valid

    if (!_requested_block) {
        if (!_peer._rating.nonExpectedMessage()) {
            // need to disconnect this peer since in has different genesis block, this is fatal
            // TODO: add more convenient way to close session in case of low rating
            _peer.endSession(msg::Close{});
        }
        return true;
    }
    else if (*_requested_block != hash) {
        if (!_peer._rating.invalidMessage()) {
            // need to disconnect this peer since in has different genesis block, this is fatal
            _peer.endSession(msg::Close{});
        }
        return true;
    }
    else {
        _requested_block.reset();
        _sync_blocks.push_back(block);
        const auto& next = block.getPrevBlockHash();
        if (next == base::Sha256::null()) {
            if (!_peer._rating.differentGenesis()) {
                // need to disconnect this peer since in has different genesis block, this is fatal
                _peer.endSession(msg::Close{});
            }
            return true;
        }
        else if (_peer._core.findBlock(next)) {
            LOG_DEBUG << "Peer " << &_peer << " applying all " << _sync_blocks.size() << " sync blocks";
            for (auto it = _sync_blocks.crbegin(); it != _sync_blocks.crend(); ++it) {
                if (_peer._core.tryAddBlock(*it) != Blockchain::AdditionResult ::ADDED) {
                    LOG_DEBUG << "Applying error";
                    break;
                }
            }
            _sync_blocks.clear();
            _sync_blocks.shrink_to_fit();
        }
        else {
            requestBlock(block.getPrevBlockHash());
        }
        return true;
    }
}


bool Peer::Synchronizer::handleReceivedNewBlock(const base::Sha256& hash, const ImmutableBlock& block)
{
    ASSERT(hash == block.getHash());

    if (_peer._core.tryAddBlock(block) == Blockchain::AdditionResult::ADDED) { // if this is a new block
        return true;
    }
    else {
        if (_requested_block) {
            _sync_blocks.push_front(block); // TODO: check if it is valid continuation for .begin()
            return true;
        }
        else {
            if (_peer._core.tryAddBlock(block) == Blockchain::AdditionResult::ADDED) {
                return true;
            }
            else {
                return false;
            }
        }
    }
}


bool Peer::Synchronizer::isSynchronised() const
{
    return !_requested_block;
}


void Peer::Synchronizer::requestBlock(base::Sha256 block_hash)
{
    if (_requested_block) {
        RAISE_ERROR(base::RuntimeError, "block was already requested and not answered yet");
    }
    else {
        _peer.requestBlock(block_hash);
        _requested_block = std::move(block_hash);
    }
}

//===============================================

void Peer::process(base::SerializationIArchive&& ia)
{
    auto msg_type = ia.deserialize<msg::Type>();
    PEER_LOG << "processing " << enumToString(msg_type);
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
        case msg::NewBlock::TYPE_ID: {
            handle(ia.deserialize<msg::NewBlock>());
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
    PEER_LOG << "Processed " << enumToString(msg_type);
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
        _non_handshaked_pool.tryRemovePeer(this);
        _requests.send(
          msg::Accepted{ _core.getThisNodeAddress(), getPublicEndpoint().getPort(), _core.getTopBlockHash() });

        requestLookup(getAddress(), base::config::NET_LOOKUP_ALPHA);
        _synchronizer.handleReceivedTopBlockHash(msg.top_block_hash);
    }
    else {
        PEER_LOG << "Handling CONNECT: sending CANNOT_ACCEPT, because can't add to pool";
        _rating.cannotAddToPool();
        endSession(
          msg::CannotAccept{ msg::CannotAccept::RefusionReason::BUCKET_IS_FULL, _host.allConnectedPeersInfo() });
    }
}


void Peer::handle(lk::msg::CannotAccept&& msg)
{
    _rating.connectionRefused();
    for (const auto& peer : msg.peers_info) {
        if (auto rating = _host.getRatingManager().get(peer.endpoint)) {
            _host.checkOutPeer(peer.endpoint, peer.address);
        }
    }
}


void Peer::handle(lk::msg::Accepted&& msg)
{
    _address = msg.address;

    _requests.send(msg::Lookup{ getAddress(), base::config::NET_LOOKUP_ALPHA });

    if (tryAddToPool()) {
        _non_handshaked_pool.tryRemovePeer(this);
        _synchronizer.handleReceivedTopBlockHash(msg.top_block_hash);
    }
    else {
        PEER_LOG << "handling of ACCEPTED: cannot add to handshaked pool";
        _rating.cannotAddToPool();
        endSession(
          msg::CannotAccept{ msg::CannotAccept::RefusionReason::BUCKET_IS_FULL, _host.allConnectedPeersInfo() });
    }
}


void Peer::handle(lk::msg::Ping&&) {}


void Peer::handle(lk::msg::Pong&&) {}


void Peer::handle(lk::msg::Lookup&& msg)
{
    auto reply = _handshaked_pool.lookup(msg.address, msg.selection_size);
    _requests.send(msg::LookupResponse{ msg.address, std::move(reply) });
}


void Peer::handle(lk::msg::LookupResponse&& msg)
{
    // either we continue to ask for closest nodes or just connect to them
    // TODO: a peer table, where we ask for LOOKUP, and collect their responds + change the very beginning of
    // communication: now it is not necessary to do a HANDSHAKE if we just want to ask for LOOKUP

    if constexpr (base::config::IS_DEBUG) {
        std::ostringstream ss;
        ss << "Lookup response peer entries:\n";
        for (const auto& pi : msg.peers_info) {
            ss << '\t' << pi.endpoint << ' ' << pi.address << '\n';
        }
        LOG_DEBUG << ss.str();
    }

    for (const auto& pi : msg.peers_info) {
        if (auto rating = _host.getRatingManager().get(pi.endpoint)) {
            LOG_DEBUG << "Checking out peer " << pi.endpoint << " (address = " << pi.address << ')';
            _host.checkOutPeer(pi.endpoint, pi.address);
        }
    }
}


void Peer::handle(lk::msg::Transaction&& msg)
{
    _core.addPendingTransaction(msg.tx);
}


void Peer::handle(lk::msg::GetBlock&& msg)
{
    PEER_LOG << "Received GET_BLOCK on " << msg.block_hash;
    if (auto block = _core.findBlock(msg.block_hash)) {
        _requests.send(msg::Block{ msg.block_hash, *block });
    }
    else {
        _requests.send(msg::BlockNotFound{ msg.block_hash });
    }
}


void Peer::handle(lk::msg::Block&& msg)
{
    PEER_LOG << "handling received " << msg.block_hash << " block";
    if (msg.block_hash != base::Sha256::compute(base::toBytes(msg.block))) {
        PEER_LOG << "invalid message";
        _rating.invalidMessage();
        return;
    }

    _synchronizer.handleReceivedBlock(msg.block_hash, msg.block);
}


void Peer::handle(lk::msg::BlockNotFound&& msg)
{
    LOG_DEBUG << "Block not found " << msg.block_hash;
}


void Peer::handle(lk::msg::NewBlock&& msg)
{
    PEER_LOG << "handling received " << msg.block_hash << " block";
    if (msg.block_hash != base::Sha256::compute(base::toBytes(msg.block))) { // TODO: use checksum
        PEER_LOG << "invalid message";
        _rating.invalidMessage();
        return;
    }

    _synchronizer.handleReceivedNewBlock(msg.block_hash, msg.block);
}


void Peer::handle(lk::msg::Close&& msg)
{
    detachFromPools();
    _session->close();
}


//===============================================


Requests::SessionHandler::SessionHandler(Requests& requests)
  : _r{ requests }
{}


void Requests::SessionHandler::onReceive(const base::Bytes& bytes)
{
    _r.onMessageReceive(bytes);
}


void Requests::SessionHandler::onClose()
{
    _r._active_requests.clear();
    _r.onClose();
}


Request::Request(base::OwningPool<Request>& active_requests,
                 MessageId id,
                 ResponseCallback response_callback,
                 TimeoutCallback timeout_callback,
                 boost::asio::io_context& io_context)
  : _active_requests{ active_requests }
  , _id{ id }
  , _response_callback{ std::move(response_callback) }
  , _timeout_callback{ std::move(timeout_callback) }
  , _timer{ io_context }
{
    _timer.expires_after(std::chrono::seconds(base::config::NET_PING_FREQUENCY));
    _timer.async_wait([request_weak = weak_from_this()](const boost::system::error_code& ec) {
        if (auto r = request_weak.lock()) {
            if (!r->_is_already_processed.test_and_set()) {
                r->_active_requests.disown(r.get());
                r->_timeout_callback();
            }
        }
    });
}


Request::MessageId Request::getId() const noexcept
{
    return _id;
}


void Request::runCallback(base::SerializationIArchive&& ia)
{
    if (!_is_already_processed.test_and_set()) {
        _response_callback(std::move(ia));
    }
}


Requests::Requests(std::weak_ptr<net::Session> session, boost::asio::io_context& io_context)
  : _session{ std::move(session) }
  , _io_context{ io_context }
{
    if (auto s = _session.lock()) {
        _session_handler = std::make_shared<SessionHandler>(*this);
        s->setHandler(_session_handler);
    }
    else {
        RAISE_ERROR(net::ClosedSession, "Session is already closed");
    }
}


void Requests::setDefaultCallback(Request::ResponseCallback cb)
{
    _default_callback = std::move(cb);
}


void Requests::setCloseCallback(Requests::CloseCallback cb)
{
    _close_callback = std::move(cb);
}


void Requests::onMessageReceive(const base::Bytes& received_bytes)
{
    base::SerializationIArchive ia(received_bytes);
    auto msg_id = ia.deserialize<Request::MessageId>();

    bool is_handled = false;
    _active_requests.disownIf([msg_id, &is_handled, &ia](Request& request) {
        if (request.getId() == msg_id) {
            is_handled = true;
            request.runCallback(std::move(ia));
            return true;
        }
        else {
            return false;
        }
    });

    if (!is_handled && _default_callback) {
        _default_callback(std::move(ia));
    }
}


void Requests::onClose()
{
    if constexpr (base::config::IS_DEBUG) {
        if (auto l = _session.lock()) {
            LOG_DEBUG << "Processing onClose callback for " << l->getEndpoint();
        }
    }
    if (_close_callback) {
        _close_callback();
    }
}

}