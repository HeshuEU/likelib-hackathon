#include "manager.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include <boost/asio/connect.hpp>

namespace ba = boost::asio;

namespace network
{

Manager::Manager(const network::NetworkAddress& listen_ip) : _listen_ip{listen_ip}
{}


Manager::~Manager()
{
    _io_context.stop();
    waitForFinish();
}


void Manager::run()
{
    _network_thread = std::make_unique<std::thread>(&Manager::_networkThreadWorkerFunction, this);
    _acceptClients();
}


void Manager::_acceptClients()
{
    ASSERT(!_acceptor);

    using namespace ba::ip;
    _acceptor = std::make_unique<tcp::acceptor>(_io_context, static_cast<ba::ip::tcp::endpoint>(_listen_ip));
    _acceptLoop();
}


void Manager::_networkThreadWorkerFunction() noexcept
{
    try {
        _io_context.run();
    }
    catch(...) {
        // global catch done for safety, since thread function cannot throw.
        // TODO: thread worker function error-handling
    }
}


void Manager::_acceptLoop()
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
        _acceptLoop();
    });
}


void Manager::connect(const std::vector<network::NetworkAddress>& addresses)
{
    for(const auto& address : addresses) {
        connect(address);
    }
}


void Manager::connect(const network::NetworkAddress& address)
{
    auto socket = std::make_unique<ba::ip::tcp::socket>(_io_context);
    socket->async_connect(static_cast<ba::ip::tcp::endpoint>(address),
                          [this, socket = std::move(socket)](const boost::system::error_code& ec) mutable {
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
