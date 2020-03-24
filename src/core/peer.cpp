#include "peer.hpp"

#include "core/core.hpp"
#include "core/host.hpp"

namespace lk
{

Peer::Info Peer::Info::deserialize(base::SerializationIArchive& ia)
{
    auto endpoint = ia.deserialize<net::Endpoint>();
    auto address = ia.deserialize<lk::Address>();
    Peer::Info ret{std::move(endpoint), std::move(address)};
    return ret;
}


void Peer::Info::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(endpoint);
    oa.serialize(address);
}

//================================

Peer::Peer(std::unique_ptr<net::Session> session, lk::Core& core, lk::Host& host)
  : _session{ std::move(session) }
  , _core{core}
  , _protocol{ lk::Protocol::peerAccepted(lk::MessageProcessor::Context{ &core, &host, this }) }
{}


net::Endpoint Peer::getEndpoint() const
{
    return _session->getEndpoint();
}


std::optional<net::Endpoint> Peer::getPublicEndpoint() const
{
    return _endpoint_for_incoming_connections;
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


const net::Session& Peer::getSession() const noexcept
{
    return *_session;
}


void Peer::addSyncBlock(lk::Block block)
{
    _sync_blocks.push_front(std::move(block));
}


void Peer::applySyncs()
{
    for(auto&& sync : _sync_blocks) {
        _core.tryAddBlock(sync);
    }
    _sync_blocks.clear();
}


const std::forward_list<lk::Block>& Peer::getSyncBlocks() const noexcept
{
    return _sync_blocks;
}


void Peer::send(const base::Bytes& data)
{
    _session->send(data);
}


void Peer::send(base::Bytes&& data)
{
    _session->send(std::move(data));
}


std::optional<lk::Address> Peer::getAddress() const
{
    return _address;
}


void Peer::setAddress(lk::Address address)
{
    _address = std::move(address);
}


bool Peer::isClosed() const
{
    return !_session || _session->isClosed();
}


Peer::Info Peer::getInfo() const
{
    return Peer::Info{_session->getEndpoint(), (_address ? *_address : lk::Address::null())};
}

//=====================================

}