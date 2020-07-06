#include "connection.hpp"

#include "base/assert.hpp"
#include "base/bytes.hpp"
#include "base/log.hpp"


namespace websocket
{

WebSocketConnection::WebSocketConnection(boost::asio::ip::tcp::socket&& socket,
                                         ConnectionReceivedCallback request_callback,
                                         ConnectionCloseCallback close_callback)
  : _connected_endpoint{ socket.remote_endpoint() }
  , _web_socket{ std::move(socket) }
  , _is_ready{ false }
  , _request_callback{ std::move(request_callback) }
  , _close_callback{ std::move(close_callback) }
{
    ASSERT(_request_callback);
    ASSERT(_close_callback);
}


WebSocketConnection::~WebSocketConnection()
{
    LOG_DEBUG << "deleting websocket connection[ip_v4]: " << _connected_endpoint;
    do_close();
}


void WebSocketConnection::accept()
{
    ASSERT(!_is_ready);

    _web_socket.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));
    static const std::string server_name{ std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async" };
    _web_socket.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::response_type& res) { res.set(boost::beast::http::field::server, server_name); }));

    _web_socket.async_accept(boost::beast::bind_front_handler(&WebSocketConnection::on_accept, shared_from_this()));
}


void WebSocketConnection::on_accept(boost::beast::error_code ec)
{
    if (ec) {
        LOG_DEBUG << "websocket accept error: " << ec.message();
        return;
    }
    _is_ready = true;
    do_read();
}


void WebSocketConnection::do_read()
{
    if (_is_ready) {
        _web_socket.async_read(_read_buffer,
                               boost::beast::bind_front_handler(&WebSocketConnection::on_read, shared_from_this()));
    }
}


void WebSocketConnection::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec) {
        LOG_DEBUG << "read error by reason: " << ec.message();
        return do_close();
    }

    base::Bytes received_bytes(bytes_transferred);
    std::memcpy(received_bytes.getData(), _read_buffer.data().data(), bytes_transferred);
    _read_buffer.clear();
    do_read();

    base::PropertyTree query_json;
    try {
        query_json = base::parseJson(received_bytes.toString());
    }
    catch (const base::InvalidArgument& error) {
        LOG_DEBUG << "parse query json error: " << error;
        return;
    }

    if (_is_ready) {
        _request_callback(std::move(query_json));
    }
}


void WebSocketConnection::write(base::PropertyTree&& response)
{
    if (_is_ready) {
        _web_socket.async_write(boost::asio::buffer(response.toString()),
                                boost::beast::bind_front_handler(&WebSocketConnection::on_write, shared_from_this()));
    }
}


void WebSocketConnection::close()
{
    do_close();
}


void WebSocketConnection::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        LOG_DEBUG << "Write error: " << ec.message();
        return do_close();
    }
}


void WebSocketConnection::do_close()
{
    if (_is_ready) {
        _is_ready = false;
        LOG_DEBUG << "Process closing connection[ip_v4]: " << _connected_endpoint;
        if (_close_callback) {
            try {
                _close_callback();
            }
            catch (...) {
                LOG_ERROR << "unexpected error at closing callback";
            }
        }
    }
}

}