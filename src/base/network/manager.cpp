#include "manager.hpp"

#include "base/log.hpp"

namespace ba = boost::asio;

namespace base::network
{

void Manager::run()
{
    _network_thread = std::make_unique<std::thread>(&Manager::_networkThreadWorkerFunction, this);
}

void Manager::acceptClients(const boost::asio::ip::tcp::endpoint& listen_ip)
{
    using namespace ba::ip;
    if(!_acceptor) {
        _acceptor = std::make_unique<tcp::acceptor>(_io_context, listen_ip);
        std::shared_ptr<Connection> connection_to_accept_to{new Connection{_io_context}};
        _acceptor->async_accept(
            connection_to_accept_to->socket,
            std::bind(&Manager::_acceptHandler, this, connection_to_accept_to, std::placeholders::_1));
    }
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

void Manager::_acceptHandler(std::shared_ptr<Connection> connection, const boost::system::error_code& ec)
{
    if(ec) {
        LOG_WARNING << "Connection accept failed: " << ec;
    }
    else {
        LOG_INFO << "Connection accepted: " << connection->socket.remote_endpoint().address().to_string() << ':'
                 << connection->socket.remote_endpoint().port();
    }
}

void Manager::waitForFinish()
{
    if(_network_thread && _network_thread->joinable()) {
        _network_thread->join();
        _network_thread.reset(nullptr);
    }
}

} // namespace base::network