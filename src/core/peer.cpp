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
 *      a) ACCEPTED response means that current peer was added to PeerTable of peers, that we are connecting to.
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
 *  1) Initiated during handshake: if peers top-block if futher then ours, then we request all blocks that we're lack
 * of. 2) All blocks are applied sequentally. 3) When initial top-block is applied, we say that host is synchronised
 * with given peer.
 *
 *
 *  Fix: do a synchronisation during runtime.
 */

std::vector<Peer::IdentityInfo> allPeersInfoExcept(lk::PeerPoolBase& host, const lk::Address& address)
{
    auto ret = host.allPeersInfo();
    ret.erase(std::find_if(ret.begin(), ret.end(), [address](const auto& cand) { return cand.address == address; }));
    return ret;
}

//============================================

namespace msg
{

void Connect::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(peer_id);
}


Connect Connect::deserialize(base::SerializationIArchive& ia)
{
    auto peers_id = ia.deserialize<Peer::IdentityInfo>();
    return Connect{ std::move(peers_id) };
}


void CannotAccept::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(why_not_accepted);
    oa.serialize(peers_info);
}


CannotAccept CannotAccept::deserialize(base::SerializationIArchive& ia)
{
    auto why_not_accepted = ia.deserialize<CannotAccept::RefusionReason>();
    auto peers_info = ia.deserialize<std::vector<Peer::IdentityInfo>>();
    return CannotAccept{ std::move(why_not_accepted), std::move(peers_info) };
}


void Accepted::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(theirs_top_block);
    oa.serialize(address);
    oa.serialize(public_port);
    oa.serialize(known_peers);
}


Accepted Accepted::deserialize(base::SerializationIArchive& ia)
{
    auto top_block = ia.deserialize<lk::Block>();
    auto address = ia.deserialize<lk::Address>();
    auto public_port = ia.deserialize<std::uint16_t>();
    auto known_peers = ia.deserialize<std::vector<lk::Peer::IdentityInfo>>();
    return Accepted{ std::move(top_block), std::move(address), public_port, std::move(known_peers) };
}


void AcceptedResponse::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(TYPE_ID);
    oa.serialize(theirs_top_block);
    oa.serialize(address);
    oa.serialize(public_port);
    oa.serialize(known_peers);
}


AcceptedResponse AcceptedResponse::deserialize(base::SerializationIArchive& ia)
{
    auto top_block = ia.deserialize<lk::Block>();
    auto address = ia.deserialize<lk::Address>();
    auto public_port = ia.deserialize<std::uint16_t>();
    auto known_peers = ia.deserialize<std::vector<lk::Peer::IdentityInfo>>();
    return AcceptedResponse{ std::move(top_block), std::move(address), public_port, std::move(known_peers) };
}


void Ping::serialize(base::SerializationOArchive&) const {}


Ping Ping::deserialize(base::SerializationIArchive&)
{
    return Ping{};
}


void Pong::serialize(base::SerializationOArchive&) const {}


Pong Pong::deserialize(base::SerializationIArchive&)
{
    return Pong{};
}


void Lookup::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(address);
    oa.serialize(selection_size);
}


Lookup Lookup::deserialize(base::SerializationIArchive& ia)
{
    auto address = ia.deserialize<lk::Address>();
    auto selection_size = ia.deserialize<std::uint8_t>();
    return Lookup{ std::move(address), selection_size };
}


void LookupResponse::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(address);
    oa.serialize(peers_info);
}


LookupResponse LookupResponse::deserialize(base::SerializationIArchive& ia)
{
    auto address = ia.deserialize<lk::Address>();
    auto peers_info = ia.deserialize<std::vector<Peer::IdentityInfo>>();
    return LookupResponse{ std::move(address), std::move(peers_info) };
}


void Transaction::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(tx);
}


Transaction Transaction::deserialize(base::SerializationIArchive& ia)
{
    auto tx = lk::Transaction::deserialize(ia);
    return Transaction{ std::move(tx) };
}


void GetBlock::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(block_hash);
}


GetBlock GetBlock::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return GetBlock{ std::move(block_hash) };
}


void Block::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(block);
}


Block Block::deserialize(base::SerializationIArchive& ia)
{
    auto block = lk::Block::deserialize(ia);
    return Block{ std::move(block) };
}


void BlockNotFound::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(block_hash);
}


BlockNotFound BlockNotFound::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return BlockNotFound{ std::move(block_hash) };
}


void Close::serialize(base::SerializationOArchive&) const {}


Close Close::deserialize(base::SerializationIArchive&)
{
    return Close{};
}

}

//============================================

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


void Peer::sendBlock(const lk::Block& block)
{
    msg::Block msg{ block };
    sendMessage(msg);
}


void Peer::sendTransaction(const lk::Transaction& tx)
{
    msg::Transaction msg{ tx };
    sendMessage(msg);
}


void Peer::lookup(const lk::Address& address,
                  const std::uint8_t alpha,
                  std::function<void(std::vector<Peer::IdentityInfo>)> callback)
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

    data->timer.async_wait([this, data](const auto& ec) {
        data->was_responded = true;
        _lookup_callbacks.erase(_lookup_callbacks.find(data->address));
    });

    _lookup_callbacks.insert(
      { address, [data, callback = std::move(callback)](auto peers_info) { callback(peers_info); } });

    sendMessage(msg::Lookup{ address, alpha }, {});
}


std::shared_ptr<Peer> Peer::accepted(std::shared_ptr<net::Session> session, lk::Host& host, lk::Core& core)
{
    std::shared_ptr<Peer> ret(new Peer(std::move(session), false, host.getIoContext(), host.getPool(), core, host));
    ret->startSession();
    return ret;
}


std::shared_ptr<Peer> Peer::connected(std::shared_ptr<net::Session> session, lk::Host& host, lk::Core& core)
{
    std::shared_ptr<Peer> ret(new Peer(std::move(session), true, host.getIoContext(), host.getPool(), core, host));
    ret->startSession();
    return ret;
}


Peer::Peer(std::shared_ptr<net::Session> session,
           bool is_connected,
           boost::asio::io_context& io_context,
           lk::PeerPoolBase& pool,
           lk::Core& core,
           lk::Host& host)
  : _session{ std::move(session) }
  , _io_context{ io_context }
  , _address{ lk::Address::null() }
  , _was_connected_to{ is_connected }
  , _pool{ pool }
  , _core{ core }
  , _host{ host }
{
    _session->setHandler(std::make_unique<Handler>(*this));
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
            sendMessage(msg::Accepted{ _core.getTopBlock(),
                                       _core.getThisNodeAddress(),
                                       getPublicEndpoint().getPort(),
                                       allPeersInfoExcept(_pool, getAddress()) });
        }
        else {
            sendMessage(msg::CannotAccept{ msg::CannotAccept::RefusionReason::BUCKET_IS_FULL, _pool.allPeersInfo() });
            // and close peer properly
        }
    }
}


bool Peer::isClosed() const
{
    return !_session || _session->isClosed();
}


Peer::IdentityInfo Peer::getInfo() const
{
    return Peer::IdentityInfo{ _session->getEndpoint(), _address };
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


Peer::Handler::Handler(Peer& peer)
  : _peer{ peer }
{}


void Peer::Handler::onReceive(const base::Bytes& bytes)
{
    _peer.process(bytes);
}


void Peer::Handler::onClose()
{
    if (_peer._is_attached_to_pool) {
        _peer._pool.removePeer(&_peer);
    }
}


void Peer::process(const base::Bytes& received_data)
{
    base::SerializationIArchive ia(received_data);
    auto msg_type = ia.deserialize<msg::Type>();
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
        case msg::AcceptedResponse::TYPE_ID: {
            handle(ia.deserialize<msg::AcceptedResponse>());
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

            // invalid message type received
            // TODO: reduce peer rating
        }
    }
}


void Peer::handle(lk::msg::Connect&& msg)
{
    if (tryAddToPool()) {
        //        AcceptedMessage msg{};
        //        sendMessage(msg);
    }
    else {
        // send(prepareMessage<CannotAcceptMessage>());
    }
}


void Peer::handle(lk::msg::CannotAccept&& msg)
{
    _pool.removePeer(this); // ???!!!

    for (const auto& peer : msg.peers_info) {
        _host.checkOutPeer(peer.endpoint, [](std::shared_ptr<Peer> peer) {
            if (peer)
                peer->startSession();
        });
    }
}


void Peer::handle(lk::msg::Accepted&& msg)
{
    const auto& ours_top_block = _core.getTopBlock();

    sendMessage(msg::AcceptedResponse{ _core.getTopBlock(),
                                       _core.getThisNodeAddress(),
                                       getPublicEndpoint().getPort(),
                                       allPeersInfoExcept(_host.getPool(), getAddress()) },
                {});

    if (msg.public_port) {
        auto public_ep = getEndpoint();
        public_ep.setPort(msg.public_port);
        setServerEndpoint(public_ep);
    }

    for (const auto& peer_info : msg.known_peers) {
        _host.checkOutPeer(peer_info.endpoint, [](std::shared_ptr<Peer> peer) {
            if (peer) {
                peer->startSession();
            }
        });
    }

    if (msg.theirs_top_block == ours_top_block) {
        setState(lk::Peer::State::SYNCHRONISED);
        return; // nothing changes, because top blocks are equal
    }
    else {
        if (ours_top_block.getDepth() > msg.theirs_top_block.getDepth()) {
            setState(lk::Peer::State::SYNCHRONISED);
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            if (_core.getTopBlock().getDepth() + 1 == msg.theirs_top_block.getDepth()) {
                _core.tryAddBlock(msg.theirs_top_block);
                setState(lk::Peer::State::SYNCHRONISED);
            }
            else {
                base::SerializationOArchive oa;
                sendMessage(msg::GetBlock{ msg.theirs_top_block.getPrevBlockHash() });
                setState(lk::Peer::State::REQUESTED_BLOCKS);
                addSyncBlock(std::move(msg.theirs_top_block));
            }
        }
    }
}


void Peer::handle(lk::msg::AcceptedResponse&& msg)
{
    const auto& ours_top_block = _core.getTopBlock();

    sendMessage(msg::AcceptedResponse{ _core.getTopBlock(),
                                       _core.getThisNodeAddress(),
                                       getPublicEndpoint().getPort(),
                                       allPeersInfoExcept(_host.getPool(), getAddress()) },
                {});

    if (msg.public_port) {
        auto public_ep = getEndpoint();
        public_ep.setPort(msg.public_port);
        setServerEndpoint(public_ep);
    }

    for (const auto& peer_info : msg.known_peers) {
        _host.checkOutPeer(peer_info.endpoint, [](std::shared_ptr<Peer> peer) {
            if (peer) {
                peer->startSession();
            }
        });
    }

    if (msg.theirs_top_block == ours_top_block) {
        setState(lk::Peer::State::SYNCHRONISED);
        return; // nothing changes, because top blocks are equal
    }
    else {
        if (ours_top_block.getDepth() > msg.theirs_top_block.getDepth()) {
            setState(lk::Peer::State::SYNCHRONISED);
            // do nothing, because we are ahead of this peer and we don't need to sync: this node might sync
            return;
        }
        else {
            if (_core.getTopBlock().getDepth() + 1 == msg.theirs_top_block.getDepth()) {
                _core.tryAddBlock(msg.theirs_top_block);
                setState(lk::Peer::State::SYNCHRONISED);
            }
            else {
                base::SerializationOArchive oa;
                sendMessage(msg::GetBlock{ msg.theirs_top_block.getPrevBlockHash() });
                setState(lk::Peer::State::REQUESTED_BLOCKS);
                addSyncBlock(std::move(msg.theirs_top_block));
            }
        }
    }
}


void Peer::handle(lk::msg::Ping&&) {}


void Peer::handle(lk::msg::Pong&&) {}


void Peer::handle(lk::msg::Lookup&& msg)
{
    auto reply = _pool.lookup(msg.address, msg.selection_size);
    sendMessage(msg::LookupResponse{ msg.address, std::move(reply) });
}


void Peer::handle(lk::msg::LookupResponse&& msg)
{
    // either we continue to ask for closest nodes or just connect to them
    // TODO: a peer table, where we ask for LOOKUP, and collect their responds + change the very beginning of
    // communication: now it is not necessary to do a HANDSHAKE if we just want to ask for LOOKUP

    if (auto it = _lookup_callbacks.find(_address); it != _lookup_callbacks.end()) {
        auto callback = std::move(it->second);
        _lookup_callbacks.erase(it);
        callback(msg.peers_info);
    }
}


void Peer::handle(lk::msg::Transaction&& msg)
{
    _core.addPendingTransaction(msg.tx);
}


void Peer::handle(lk::msg::GetBlock&& msg)
{
    LOG_DEBUG << "Received GET_BLOCK on " << msg.block_hash;
    auto block = _core.findBlock(msg.block_hash);
    if (block) {
        sendMessage(msg::Block{ (*block) });
    }
    else {
        sendMessage(msg::BlockNotFound{ msg.block_hash });
    }
}


void Peer::handle(lk::msg::Block&& msg)
{
    if (getState() == lk::Peer::State::SYNCHRONISED) {
        // we're synchronised already

        if (_core.tryAddBlock(msg.block)) {
            // block added, all is OK
        }
        else {
            // in this case we are missing some blocks
        }
    }
    else {
        // we are in synchronization process
        lk::BlockDepth block_depth = msg.block.getDepth();
        addSyncBlock(std::move(msg.block));

        if (block_depth == _core.getTopBlock().getDepth() + 1) {
            applySyncs();
        }
        else {
            sendMessage(msg::GetBlock{ getSyncBlocks().front().getPrevBlockHash() });
        }
    }
}


void Peer::handle(lk::msg::BlockNotFound&& msg)
{
    LOG_DEBUG << "Block not found " << msg.block_hash;
}


void Peer::handle(lk::msg::Close&& msg) {}

}