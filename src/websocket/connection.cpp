#include "connection.hpp"
#include "tools.hpp"

#include "base/assert.hpp"
#include "base/bytes.hpp"
#include "base/log.hpp"

namespace websocket
{

WebSocketConnection::WebSocketConnection(boost::asio::ip::tcp::socket&& socket,
                                         ProcessRequestCallback request_callback,
                                         ConnectionCloseCallback close_callback)
  : _connected_endpoint{ socket.remote_endpoint() }
  , _websocket{ std::move(socket) }
  , _is_ready{ false }
  , _process_callback{ std::move(request_callback) }
  , _close_callback{ std::move(close_callback) }
{
    LOG_TRACE << "creating websocket connection " << _connected_endpoint;
    ASSERT(_process_callback);
    ASSERT(_close_callback);
}


WebSocketConnection::~WebSocketConnection() noexcept
{
    LOG_TRACE << "deleting websocket connection " << _connected_endpoint;
    doClose();
}


void WebSocketConnection::accept()
{
    _websocket.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

    static const std::string server_name{ std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async" };
    _websocket.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::response_type& res) { res.set(boost::beast::http::field::server, server_name); }));

    _websocket.async_accept(boost::beast::bind_front_handler(&WebSocketConnection::onAccept, shared_from_this()));
}


void WebSocketConnection::onAccept(boost::beast::error_code ec)
{
    if (ec) {
        LOG_DEBUG << "websocket accept error: " << ec.message();
        return;
    }
    LOG_TRACE << "websocket connection accepted " << _connected_endpoint;
    doRead();
}


void WebSocketConnection::doRead()
{
    _websocket.async_read(_read_buffer,
                          boost::beast::bind_front_handler(&WebSocketConnection::onRead, shared_from_this()));
}


void WebSocketConnection::onRead(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec) {
        LOG_DEBUG << "read error by reason: " << ec.message();
        return;
    }

    base::Bytes receivedBytes(bytes_transferred);
    std::memcpy(receivedBytes.getData(), _read_buffer.data().data(), bytes_transferred);
    _read_buffer.clear();
    doRead();

    base::json::Value query;
    try {
        query = base::json::Value::parse(receivedBytes.toString());
    }
    catch (const std::exception& ex) {
        LOG_DEBUG << "parse query json error at connection " << _connected_endpoint << " error: " << ex.what();
        return;
    }
    catch (...) {
        LOG_DEBUG << "parse query json error at connection " << _connected_endpoint;
        return;
    }

    _process_callback(std::move(query));
}


void WebSocketConnection::write(base::json::Value response)
{
    auto output = response.serialize();
    boost::beast::error_code ec;
    _websocket.write(boost::asio::buffer(output), ec);
    if (ec) {
        LOG_DEBUG << "write error at connection " << _connected_endpoint << " by reason: " << ec.message();
        return;
    }
}


void WebSocketConnection::close()
{
    doClose();
}


void WebSocketConnection::doClose() noexcept
{
    try {
        LOG_TRACE << "closing connection " << _connected_endpoint;
        boost::beast::error_code ec;
        _websocket.close(boost::beast::websocket::close_code::normal, ec);
        if (ec) {
            LOG_DEBUG << "websocket close error: " << ec.message();
        }
    }
    catch (...) {
        LOG_ERROR << "unexpected error at websocket connection closing " << _connected_endpoint;
    }

    try {
        _close_callback();
    }
    catch (...) {
        LOG_ERROR << "unexpected error at closing callback for connection " << _connected_endpoint;
    }
}

}