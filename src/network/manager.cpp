#include "manager.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include <boost/asio/connect.hpp>

#include <chrono>

namespace ba = boost::asio;

namespace network
{

Manager::Manager(const network::NetworkAddress& listen_ip) : _listen_ip{listen_ip}, _heartbeatTimer{_io_context}
{}


Manager::~Manager()
{
    _io_context.stop();
    waitForFinish();
}


void Manager::run()
{
    acceptClients();
    scheduleHeartBeat();
    _network_thread = std::make_unique<std::thread>(&Manager::networkThreadWorkerFunction, this);
}


void Manager::scheduleHeartBeat()
{
    for(auto it = _connections.begin(); it != _connections.end(); ) {
        if(_not_responded_peers.find((*it)->getRemoteNetworkAddress()) != _not_responded_peers.end()) {
            it = _connections.erase(it);
        }
        else {
            ++it;
        }
    }

    for(const auto& connection : _connections) {
        connection->ping([this] {
            // _not_responded_peers.erase(connection->getRemoteNetworkAddress());
        });
    }

    _heartbeatTimer.expires_after(std::chrono::seconds(base::config::NETWORK_PING_FREQUENCY));
    _heartbeatTimer.async_wait([this]{

    });
}


void Manager::acceptClients()
{
    ASSERT(!_acceptor);

    using namespace ba::ip;
    LOG_INFO << "Listening on " << _listen_ip.toString();
    _acceptor = std::make_unique<tcp::acceptor>(_io_context, static_cast<ba::ip::tcp::endpoint>(_listen_ip));
    _acceptor->set_option(ba::socket_base::reuse_address(true));
    acceptLoop();
}


void Manager::networkThreadWorkerFunction() noexcept
{
    try {
        _io_context.run();
    }
    catch(...) {
        // global catch done for safety, since thread function cannot throw.
        // TODO: thread worker function error-handling
    }
}


void Manager::acceptLoop()
{
    _acceptor->async_accept([this](const boost::system::error_code& ec, ba::ip::tcp::socket socket) {
        if(ec) {
            LOG_WARNING << "Connection accept failed: " << ec;
        }
        else {
            auto connection = std::make_unique<Connection>(_io_context, std::move(socket));
            LOG_INFO << "Connection accepted: " << connection->getRemoteNetworkAddress().toString();
            connection->startSession();
            _connections.push(std::move(connection));
        }
        acceptLoop();
    });
}


void Manager::connect(const std::vector<network::NetworkAddress>& addresses)
{
    for(const auto& address: addresses) {
        connect(address);
    }
}


void Manager::connect(const network::NetworkAddress& address)
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
                              LOG_INFO << "Connection established: "
                                       << connection->getRemoteNetworkAddress().toString();
                              connection->startSession();
                              _connections.push(std::move(connection));
                          });
}


void Manager::waitForFinish()
{
    if(_network_thread && _network_thread->joinable()) {
        _network_thread->join();
        _network_thread.reset(nullptr);
    }
}

} // namespace network
