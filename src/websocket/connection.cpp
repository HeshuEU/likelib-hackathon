#include "connection.hpp"

#include "base/assert.hpp"
#include "base/bytes.hpp"
#include "base/log.hpp"

namespace websocket
{

WebSocketConnection::WebSocketConnection(boost::asio::ip::tcp::socket&& socket,
                                         ProcessRequestCallback requestCallback,
                                         ConnectionCloseCallback closeCallback)
  : _connectedEndpoint{ socket.remote_endpoint() }
  , _websocket{ std::move(socket) }
  , _is_ready{ false }
  , _process{ std::move(requestCallback) }
  , _closed{ std::move(closeCallback) }
{
    ASSERT(_process);
    ASSERT(_closed);
}


WebSocketConnection::~WebSocketConnection() noexcept
{
    LOG_DEBUG << "deleting websocket connection[ip_v4]: " << _connectedEndpoint;
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
    doRead();
}


void WebSocketConnection::doRead()
{
    _websocket.async_read(_readBuffer,
                          boost::beast::bind_front_handler(&WebSocketConnection::onRead, shared_from_this()));
}


void WebSocketConnection::onRead(boost::beast::error_code ec, std::size_t bytesTransferred)
{
    if (ec) {
        LOG_DEBUG << "read error by reason: " << ec.message();
        return;
    }

    base::Bytes receivedBytes(bytesTransferred);
    std::memcpy(receivedBytes.getData(), _readBuffer.data().data(), bytesTransferred);
    _readBuffer.clear();
    doRead();

    base::PropertyTree queryJson;
    try {
        queryJson = base::parseJson(receivedBytes.toString());
    }
    catch (const base::InvalidArgument& error) {
        LOG_DEBUG << "parse query json error: " << error.what();
        return;
    }

    _process(std::move(queryJson));
}


void WebSocketConnection::write(base::PropertyTree&& response)
{
    boost::beast::error_code ec;
    _websocket.write(boost::asio::buffer(response.toString()), ec);
    if (ec) {
        LOG_DEBUG << "write error by reason: " << ec.message();
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
        LOG_DEBUG << "Process closing connection[ip_v4]: " << _connectedEndpoint;
        boost::beast::error_code ec;
        _websocket.close(boost::beast::websocket::close_code::normal, ec);
        if (ec) {
            LOG_DEBUG << "websocket close error: " << ec.message();
        }
    }
    catch (...) {
        LOG_ERROR << "unexpected error at websocket closing";
    }

    if (_closed) {
        try {
            _closed();
        }
        catch (...) {
            LOG_ERROR << "unexpected error at closing callback";
        }
    }
}

}