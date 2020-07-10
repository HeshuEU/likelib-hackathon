#include "acceptor.hpp"
#include "error.hpp"
#include "tools.hpp"

#include "base/log.hpp"

#include <boost/asio.hpp>


namespace websocket
{

WebSocketAcceptor::WebSocketAcceptor(const base::PropertyTree& config, SocketRegistration registration)
  : _config{ config }
  , _connectionRegistration{ std::move(registration) }
  , _io_context{}
  , _endpoint{ createEndpoint(_config.get<std::string>("websocket.listen_addr")) }
  , _acceptor{ _io_context }
{
    ASSERT(_connectionRegistration);

    boost::beast::error_code ec;

    _acceptor.open(_endpoint.protocol(), ec);
    if (ec) {
        LOG_ERROR << "open error: " << ec.message();
        RAISE_ERROR(SetUpError, ec.message());
    }

    _acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) {
        LOG_ERROR << "set up error: " << ec.message();
        RAISE_ERROR(SetUpError, ec.message());
    }

    _acceptor.bind(_endpoint, ec);
    if (ec) {
        LOG_ERROR << "bind error: " << ec.message();
        RAISE_ERROR(SetUpError, ec.message());
    }

    _acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        LOG_ERROR << "listen error: " << ec.message();
        RAISE_ERROR(SetUpError, ec.message());
    }
}


WebSocketAcceptor::~WebSocketAcceptor() noexcept
{
    stop();
}


void WebSocketAcceptor::run()
{
    accept();
    _inputNetworkThread = boost::thread(boost::bind(&WebSocketAcceptor::networkThreadWorkerFunction, this));
}


void WebSocketAcceptor::stop() noexcept
{
    try {
        if (_inputNetworkThread.joinable()) {
            _io_context.stop();
            _inputNetworkThread.join();
        }
    }
    catch (const std::exception& ex) {
        LOG_ERROR << "exception at websocket acceptor stopping: " << ex.what();
    }
    catch (...) {
        LOG_ERROR << "undefinde exception at websocket acceptor stopping";
    }
}


void WebSocketAcceptor::accept()
{
    _acceptor.async_accept([this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket) {
        if (ec) {
            LOG_WARNING << "Connection accept failed: " << ec.message();
        }
        else {
            LOG_INFO << "Connection accepted: " << socket.remote_endpoint();
            _connectionRegistration(std::move(socket));
        }
        accept();
    });
}


void WebSocketAcceptor::networkThreadWorkerFunction() noexcept
{
    try {
        _io_context.run();
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Error occurred in network thread: " << e.what();
    }
    catch (...) {
        // global catch done for safety, since thread function cannot throw.
        LOG_ERROR << "Error occurred in network thread";
    }
}

}