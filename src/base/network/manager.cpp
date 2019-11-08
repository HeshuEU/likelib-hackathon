#include "manager.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

namespace ba = boost::asio;

namespace base::network
{

Manager::~Manager()
{
    _io_context.stop();
    waitForFinish();
}


void Manager::run()
{
    _network_thread = std::make_unique<std::thread>(&Manager::_networkThreadWorkerFunction, this);
}


void Manager::acceptClients(const boost::asio::ip::tcp::endpoint& listen_ip)
{
    ASSERT(!_acceptor);

    using namespace ba::ip;
    _acceptor = std::make_unique<tcp::acceptor>(_io_context, listen_ip);
    _acceptOne();
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


void Manager::_acceptOne()
{
    _acceptor->async_accept(std::bind(&Manager::_acceptHandler, this, std::placeholders::_1, std::placeholders::_2));
}


void Manager::_acceptHandler(const boost::system::error_code& ec, ba::ip::tcp::socket socket)
{
    if(ec) {
        LOG_WARNING << "Connection accept failed: " << ec;
    }
    else {
        Connection connection(std::move(socket));
        LOG_INFO << "Connection accepted: " << connection.getRemoteNetworkAddress().toString();
        _connections.push_back(std::move(connection));
    }
    _acceptOne();
}


void Manager::waitForFinish()
{
    if(_network_thread && _network_thread->joinable()) {
        _network_thread->join();
        _network_thread.reset(nullptr);
    }
}

} // namespace base::network
