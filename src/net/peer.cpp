#include "peer.hpp"

#include "base/log.hpp"

namespace net
{

Peer::Peer(std::unique_ptr<Connection> connection) : _connection{std::move(connection)}
{}


bool Peer::isActive() const noexcept
{
    return !(isClosed());
}


bool Peer::isClosed() const noexcept
{
    return !_connection || _connection->isClosed();
}


void Peer::onConnectionReceived(Packet&& packet)
{
    switch(packet.getType()) {
        case PacketType::HANDSHAKE: {
            LOG_DEBUG << "[handshake] currently does nothing";
            break;
        }
        case PacketType::PING: {
            onPing();
            _connection->send(Packet{PacketType::PONG});
            break;
        }
        case PacketType::PONG: {
            onPong();
            break;
        }
        case PacketType::DATA: {
            // call onDataHandler
            _handler(std::move(packet));
            break;
        }
        default: {
            LOG_WARNING << "[!] Received an invalid packet";
            break;
        }
    }
}


void Peer::ping()
{
    Packet p{PacketType::PING};
    _connection->send(p);
}


void Peer::onPing()
{
    refreshLastSeen();
    Packet p{PacketType::PONG};
    _connection->send(p);
}


void Peer::onPong()
{
    refreshLastSeen();
}


void Peer::refreshLastSeen()
{
    _last_seen = base::Time::now();
}


base::Time Peer::getLastSeen() const noexcept
{
    return _last_seen;
}


void Peer::receive(Peer::ReceiveHandler handler)
{
    _connection->receive(2, [this, handler = std::move(handler)](const base::Bytes& data) {
        auto length = base::fromBytes<std::uint16_t>(data);
        _connection->receive(length, [handler = std::move(handler)](const base::Bytes& data) {
            handler(data);
        });
    });
}


void Peer::send(base::Bytes data)
{
    _connection->send(std::move(data));
}


void Peer::close()
{
    ASSERT(!_connection->isClosed());
    if(!_connection->isClosed()) {
        _connection->close();
    }
}


void Peers::add(std::shared_ptr<Peer> peer)
{
    ASSERT(peer);
    std::lock_guard lk(_state_mutex);
    _peers.push_front(std::move(peer));
    ++_size;
}


std::size_t Peers::size() const noexcept
{
    std::shared_lock lk(_state_mutex);
    return _size;
}


void Peers::forEach(std::function<void(Peer&)> f)
{
    std::lock_guard lk(_state_mutex);
    for(auto& peer: _peers) {
        f(*peer);
    }
}


void Peers::removeClosed()
{
    std::lock_guard lk(_state_mutex);
    _peers.remove_if([](const auto& peer) {
        return peer.isClosed();
    });
}

} // namespace net