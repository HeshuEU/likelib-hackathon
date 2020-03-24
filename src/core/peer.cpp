#include "peer.hpp"

namespace lk
{

Peer::Peer(std::unique_ptr<net::Session> session)
  : _session{ std::move(session) }
{
    doHandshake();
}


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


void Peer::addSyncBlock(bc::Block block)
{
    _sync_blocks.push_front(std::move(block));
}


const std::forward_list<bc::Block>& Peer::getSyncBlocks() const noexcept
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


std::optional<bc::Address> Peer::getAddress() const
{
    return _address;
}


void Peer::setAddress(bc::Address address)
{
    _address = std::move(address);
}

//=====================================

}