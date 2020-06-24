#include "server.hpp"
#include "error.hpp"
#include "tools.hpp"

#include "base/log.hpp"

#include <boost/asio.hpp>

namespace web_socket
{

WebSocketServer::WebSocketServer(const base::PropertyTree& config)
  : _config{ config }
  , _io_context{}
  , _endpoint{ create_endpoint(_config.get<std::string>("web_socket.listen_addr")) }
  , _acceptor{ _io_context }
{
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


WebSocketServer::~WebSocketServer()
{
    stop();
}


void WebSocketServer::run(RequestCall on_request)
{
    do_accept(std::move(on_request));
    _network_thread = std::thread(&WebSocketServer::networkThreadWorkerFunction, this);
}


void WebSocketServer::stop()
{
    if (_network_thread.joinable()) {
        _io_context.stop();
        _network_thread.join();
    }
}


void WebSocketServer::do_accept(RequestCall on_request)
{
    accept(std::move(on_request));
}


void WebSocketServer::accept(RequestCall on_request)
{
    _acceptor.async_accept([this, on_request_received = std::move(on_request)](const boost::system::error_code& ec,
                                                                               boost::asio::ip::tcp::socket socket) {
        if (ec) {
            LOG_WARNING << "Connection accept failed: " << ec.message();
        }
        else {
            LOG_INFO << "Connection accepted: " << socket.remote_endpoint();
            std::make_shared<WebSocketSession>(std::move(socket), on_request_received)->run();
        }
        do_accept(on_request_received);
    });
}


void WebSocketServer::networkThreadWorkerFunction() noexcept
{
    try {
        _io_context.run();
    }
    catch (const std::exception& e) {
        // TODO: thread worker function error-handling
        LOG_WARNING << "Error occurred in network thread: " << e.what();
    }
    catch (...) {
        // global catch done for safety, since thread function cannot throw.
        LOG_WARNING << "Error occurred in network thread";
    }
}

}