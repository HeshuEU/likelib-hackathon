#include "peer.hpp"

#include "core/core.hpp"
#include "core/host.hpp"

namespace lk
{

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

std::unique_ptr<Peer> Peer::accepted(std::unique_ptr<net::Session> session, lk::PeerPoolBase& host, lk::Core& core)
{
    auto ret = std::make_unique<Peer>(Peer(std::move(session), host, core));
    auto protocol = std::make_unique<lk::Protocol>(
      lk::Protocol::peerAccepted(lk::MessageProcessor::Context{ &core, &host, ret.get() }));
    ret->setProtocol(std::move(protocol));
    ret->start();
    return ret;
}


std::unique_ptr<Peer> Peer::connected(std::unique_ptr<net::Session> session, lk::PeerPoolBase& host, lk::Core& core)
{
    auto ret = std::make_unique<Peer>(Peer(std::move(session), host, core));
    auto protocol = std::make_unique<lk::Protocol>(
      lk::Protocol::peerConnected(lk::MessageProcessor::Context{ &core, &host, ret.get() }));
    ret->setProtocol(std::move(protocol));
    ret->start();
    return ret;
}


Peer::Peer(std::unique_ptr<net::Session> session, lk::PeerPoolBase& pool, lk::Core& core)
  : _session{ std::move(session) }
  , _address{ lk::Address::null() }
  , _pool{ pool }
  , _core{ core }
{
    if(_pool.tryAddPeer(shared_from_this())) {
        _is_attached_to_pool = true;
    }
    else {
        rejectedByPool();
    }
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


void Peer::setState(State new_state)
{
    _state = new_state;
}


Peer::State Peer::getState() const noexcept
{
    return _state;
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


void Peer::send(const base::Bytes& data, net::Connection::SendHandler on_send)
{
    _session->send(data, std::move(on_send));
}


void Peer::send(base::Bytes&& data, net::Connection::SendHandler on_send)
{
    _session->send(std::move(data), std::move(on_send));
}


void Peer::send(const lk::Block& block)
{
    _protocol->sendBlock(block);
}


void Peer::send(const lk::Transaction& tx)
{
    _protocol->sendTransaction(tx);
}


const lk::Address& Peer::getAddress() const noexcept
{
    return _address;
}


void Peer::setAddress(lk::Address address)
{
    _address = std::move(address);
}


void Peer::setProtocol(std::shared_ptr<lk::ProtocolBase> protocol)
{
    _protocol = std::move(protocol);
    _session->setHandler(_protocol);
}


void Peer::start()
{
    _session->start();
}


bool Peer::isClosed() const
{
    return !_session || _session->isClosed();
}


Peer::Info Peer::getInfo() const
{
    return Peer::Info{ _session->getEndpoint(), _address };
}


void Peer::rejectedByPool()
{
    _protocol->sendSessionEnd([this]{
      _session->close();
    });
}

}