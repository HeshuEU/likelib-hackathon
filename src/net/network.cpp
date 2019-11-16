#include "network.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/error.hpp>

#include <chrono>

namespace ba = boost::asio;

namespace net
{

Network::Network(const net::Endpoint& listen_ip) : _listen_ip{listen_ip}, _heartbeat_timer{_io_context}
{}


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

    for(const auto& connection : _connections) {
        _not_ponged_peer_ids.insert(connection->getId());
        connection->ping();
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
            auto connection = std::make_shared<Connection>(_io_context, std::move(socket), std::bind(&Network::connectionReceivedPacketHandler, this, std::placeholders::_1, std::placeholders::_2));
            LOG_INFO << "Connection accepted: " << connection->getEndpoint();
            connection->startSession();
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
                                  auto connection =
                                      std::make_shared<Connection>(_io_context, std::move(*socket.release()), std::bind(&Network::connectionReceivedPacketHandler, this, std::placeholders::_1, std::placeholders::_2));
                                  LOG_INFO << "Connection established: " << connection->getEndpoint();
                                  connection->startSession();
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
    for(const auto& connection : _connections) {
        if(_not_ponged_peer_ids.find(connection->getId()) != _not_ponged_peer_ids.end()) {
            connection->close();
        }
    }
    _not_ponged_peer_ids.clear();
}


void Network::connectionReceivedPacketHandler(std::shared_ptr<Connection> connection, const net::Packet& packet)
{
    switch(packet.getType()) {
        case Packet::Type::PING: {
            connection->send(Packet{Packet::Type::PONG});
            break;
        }
        case Packet::Type::PONG: {
            if(_not_ponged_peer_ids.find(connection->getId()) == _not_ponged_peer_ids.end()) {
                LOG_WARNING << "Connection " << connection->getEndpoint() << " sent an unexpected PONG";
            }
            break;
        }
        case Packet::Type::DATA: {
            break;
        }
        default: {
            LOG_WARNING << "Received an invalid packet from " << connection->getEndpoint();
        }
    }
}

} // namespace net
