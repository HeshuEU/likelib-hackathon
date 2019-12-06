#include "peer.hpp"

#include "base/log.hpp"

namespace net
{

Peer::Peer(std::shared_ptr<Connection> connection, PacketHandler handler)
    : _connection{std::move(connection)}, _handler{std::move(handler)}
{
    using namespace std::placeholders;
    _connection->startSession(std::bind(&Peer::onConnectionReceived, this, _1));
}


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


void Peer::close()
{
    if(!_connection->isClosed()) {
        _connection->close();
    }
}


//    LOG_DEBUG << "RECEIVED [" << enumToString(packet.getType()) << ']';
//    switch(packet.getType()) {
//        case PacketType::HANDSHAKE: {
//
//            break;
//        }
//        case PacketType::PONG: {
//            if(auto it = _not_ponged_peer_ids.find(connection.getId()); it == _not_ponged_peer_ids.end()) {
//                LOG_WARNING << "Connection " << connection.getEndpoint() << " sent an unexpected PONG";
//            }
//            else {
//                _not_ponged_peer_ids.erase(it);
//                connection.send(net::PacketType::DISCOVERY_REQ);
//            }
//            break;
//        }
//        case PacketType::DATA: {
//            _data_handler(std::move(packet).getData());
//            break;
//        }
//        case PacketType::DISCOVERY_REQ: {
//            // then we gotta send those node our endpoints
//            std::vector<std::string> endpoints;
//            for(const auto& c: _peers) {
//                if(c->hasServerEndpoint() && c->getEndpoint() != connection.getEndpoint()) {
//                    std::string a = c->getServerEndpoint().toString();
//                    endpoints.push_back(a);
//                }
//            }
//
//            Packet ret{net::PacketType::DISCOVERY_RES};
//            ret.setKnownEndpoints(std::move(endpoints));
//            connection.send(ret);
//
//            break;
//        }
//        case PacketType::DISCOVERY_RES: {
//            LOG_DEBUG << "Received endpoints:";
//            for(const auto& endpoint: packet.getKnownEndpoints()) {
//                LOG_DEBUG << endpoint;
//                net::Endpoint received_endpoint{endpoint};
//                // of course this will be changed later
//                bool is_found = false;
//                for(const auto& connection: _peers) {
//                    if(connection->getServerEndpoint() == received_endpoint) {
//                        is_found = true;
//                        break;
//                    }
//                }
//                if(!is_found) {
//                    LOG_DEBUG << "Going to connect to a new node: " << received_endpoint;
//                    connect(received_endpoint);
//                }
//            }
//            break;
//        }
//        default: {
//            LOG_WARNING << "Received an invalid packet from " << connection.getEndpoint();
//            break;
//        }
//    }
//}


} // namespace net