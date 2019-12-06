#include "network.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "bc/blockchain.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/error.hpp>

#include <chrono>

namespace ba = boost::asio;

namespace net
{

Network::Network(const base::PropertyTree& config, DataHandler handler)
    : _listen_ip{config.get<std::string>("net.listen_addr")},
      _server_public_port{config.get<unsigned short>("net.public_port")}, _heartbeat_timer{_io_context},
      _data_handler(handler)
{
    ASSERT(_data_handler);
}


Network::~Network()
{
    _io_context.stop();
    waitForFinish();
}


void Network::run()
{
    acceptClients();
    scheduleHeartBeat();
    _network_thread = std::make_unique<std::thread>(&Network::networkThreadWorkerFunction, this);
}


void Network::scheduleHeartBeat()
{
    ASSERT(_not_ponged_peer_ids.empty());

    for(const auto& connection: _connections) {
        _not_ponged_peer_ids.insert(connection->getId());
        connection->send({PacketType::PING});
    }

    _heartbeat_timer.expires_after(std::chrono::seconds(base::config::NET_PING_FREQUENCY));
    _heartbeat_timer.async_wait([this](const boost::system::error_code& ec) {
        dropZombieConnections();
        scheduleHeartBeat();
    });
}


void Network::acceptClients()
{
    ASSERT(!_acceptor);

    using namespace ba::ip;
    LOG_INFO << "Listening on " << _listen_ip.toString();
    _acceptor = std::make_unique<tcp::acceptor>(_io_context, static_cast<ba::ip::tcp::endpoint>(_listen_ip));
    _acceptor->set_option(ba::socket_base::reuse_address(true));
    acceptLoop();
}


void Network::networkThreadWorkerFunction() noexcept
{
    try {
        _io_context.run();
    }
    catch(...) {
        // global catch done for safety, since thread function cannot throw.
        // TODO: thread worker function error-handling
    }
}


void Network::acceptLoop()
{
    _acceptor->async_accept([this](const boost::system::error_code& ec, ba::ip::tcp::socket socket) {
        if(ec) {
            LOG_WARNING << "Connection accept failed: " << ec;
        }
        else {
            auto connection = std::make_shared<Connection>(_io_context, std::move(socket),
                std::bind(
                    &Network::onConnectionReceivedPacketHandler, this, std::placeholders::_1, std::placeholders::_2));
            LOG_INFO << "Connection accepted: " << connection->getEndpoint();
            connection->startReceivingMessages();
            _connections.push_back(std::move(connection));
        }
        acceptLoop();
    });
}


void Network::connect(const std::vector<net::Endpoint>& addresses)
{
    for(const auto& address: addresses) {
        connect(address);
    }
}


void Network::connect(const net::Endpoint& address)
{
    LOG_DEBUG << "Connecting to " << address.toString();
    auto socket = std::make_unique<ba::ip::tcp::socket>(_io_context);
    socket->async_connect(static_cast<ba::ip::tcp::endpoint>(address),
        [this, socket = std::move(socket), address](const boost::system::error_code& ec) mutable {
            if(ec) {
                switch(ec.value()) {
                    case ba::error::connection_refused: {
                        LOG_WARNING << "Connection error: host " << address.toString();
                        break;
                    }
                    case ba::error::fault: {
                        LOG_WARNING << "Connection error: invalid address";
                        break;
                    }
                    default: {
                        LOG_WARNING << "Connection error: " << ec << ' ' << ec.message();
                        break;
                    }
                }
            }
            else {
                auto connection = std::make_shared<Connection>(_io_context, std::move(*socket.release()),
                    std::bind(&Network::onConnectionReceivedPacketHandler, this, std::placeholders::_1,
                        std::placeholders::_2));
                LOG_INFO << "Connection established: " << connection->getEndpoint();

                connection->setServerEndpoint(address);
                connection->startReceivingMessages();
                net::Packet packet{net::PacketType::HANDSHAKE};
                packet.setPublicServerPort(_server_public_port);
                connection->send(packet);
                _connections.push_back(std::move(connection));
            }
        });
}



void Network::waitForFinish()
{
    if(_network_thread && _network_thread->joinable()) {
        _network_thread->join();
        _network_thread.reset(nullptr);
    }
}


void Network::dropZombieConnections()
{
    for(auto it = _connections.begin(); it != _connections.end();) {
        auto& connection = *it;
        if(_not_ponged_peer_ids.find(connection->getId()) != _not_ponged_peer_ids.end()) {
            if(!connection->isClosed()) {
                connection->close();
            }
            it = _connections.erase(it);
        }
        else {
            ++it;
        }
    }
    _not_ponged_peer_ids.clear();
}


void Network::onConnectionReceivedPacketHandler(Connection& connection, net::Packet&& packet)
{
    LOG_DEBUG << "RECEIVED [" << enumToString(packet.getType()) << ']';
    switch(packet.getType()) {
        case PacketType::HANDSHAKE: {
            LOG_DEBUG << "Received server endpoint: " << packet.getPublicServerPort();

            std::string public_ip_with_port = connection.getEndpoint().toString();
            std::string public_ip = public_ip_with_port.substr(0, public_ip_with_port.find(':'));
            connection.setServerEndpoint({public_ip, packet.getPublicServerPort()});
            break;
        }
        case PacketType::PING: {
            connection.send(Packet{PacketType::PONG});
            break;
        }
        case PacketType::PONG: {
            if(auto it = _not_ponged_peer_ids.find(connection.getId()); it == _not_ponged_peer_ids.end()) {
                LOG_WARNING << "Connection " << connection.getEndpoint() << " sent an unexpected PONG";
            }
            else {
                _not_ponged_peer_ids.erase(it);
                connection.send(net::PacketType::DISCOVERY_REQ);
            }
            break;
        }
        case PacketType::DATA: {
            _data_handler(std::move(packet).getData());
            break;
        }
        case PacketType::DISCOVERY_REQ: {
            // then we gotta send those node our endpoints
            std::vector<std::string> endpoints;
            for(const auto& c: _connections) {
                if(c->hasServerEndpoint() && c->getEndpoint() != connection.getEndpoint()) {
                    std::string a = c->getServerEndpoint().toString();
                    endpoints.push_back(a);
                }
            }

            Packet ret{net::PacketType::DISCOVERY_RES};
            ret.setKnownEndpoints(std::move(endpoints));
            connection.send(ret);

            break;
        }
        case PacketType::DISCOVERY_RES: {
            LOG_DEBUG << "Received endpoints:";
            for(const auto& endpoint: packet.getKnownEndpoints()) {
                LOG_DEBUG << endpoint;
                net::Endpoint received_endpoint{endpoint};
                // of course this will be changed later
                bool is_found = false;
                for(const auto& connection: _connections) {
                    if(connection->getServerEndpoint() == received_endpoint) {
                        is_found = true;
                        break;
                    }
                }
                if(!is_found) {
                    LOG_DEBUG << "Going to connect to a new node: " << received_endpoint;
                    connect(received_endpoint);
                }
            }
            break;
        }
        default: {
            LOG_WARNING << "Received an invalid packet from " << connection.getEndpoint();
            break;
        }
    }
}


void Network::broadcast(const base::Bytes& data)
{
    Packet p;
    p.setType(PacketType::DATA);
    p.setData(data);

    for(auto& c: _connections) {
        c->send(packet);
    }
}

} // namespace net
