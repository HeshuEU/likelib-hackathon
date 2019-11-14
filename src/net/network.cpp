#include "network.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include <boost/asio/connect.hpp>

#include <chrono>

namespace ba = boost::asio;

namespace net
{

Network::Network(const net::Endpoint& listen_ip) : _listen_ip{listen_ip}, _heartbeatTimer{_io_context}
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
    ASSERT(_not_responded_peers.empty());

    for(const auto& connection: _connections) {
        _not_responded_peers.push_back(connection->getEndpoint());
    }

    for(const auto& connection: _connections) {
        connection->ping([this, ep = connection->getEndpoint()] {
            _not_responded_peers.remove(ep);
        });
    }

    _heartbeatTimer.expires_after(std::chrono::seconds(base::config::NET_PING_FREQUENCY));
    _heartbeatTimer.async_wait([this](const boost::system::error_code& ec) {
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
            auto connection = std::make_unique<Connection>(_io_context, std::move(socket));
            LOG_INFO << "Connection accepted: " << connection->getEndpoint().toString();
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
                          [this, socket = std::move(socket)](const boost::system::error_code& ec) mutable {
                              if(ec) {
                                  // TODO: do something
                                  LOG_WARNING << "Error occurred during connect: " << ec;
                              }
                              auto connection = std::make_unique<Connection>(_io_context, std::move(*socket.release()));
                              LOG_INFO << "Connection established: " << connection->getEndpoint().toString();
                              connection->startSession();
                              _connections.push_back(std::move(connection));
                          });
}


void Network::waitForFinish()
{
    if(_network_thread && _network_thread->joinable()) {
        _network_thread->join();
        _network_thread.reset(nullptr);
    }
}


void Network::dropConnectionByEndpoint(const net::Endpoint& endpoint)
{
    _connections.remove_if([&endpoint](const auto& connection) {
        return connection->getEndpoint() == endpoint;
    });
}


void Network::dropZombieConnections()
{
    // TODO: optimize
    for(const auto& ep: _not_responded_peers) {
        dropConnectionByEndpoint(ep);
    }
    _not_responded_peers.clear();
}

} // namespace net
