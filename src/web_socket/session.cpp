#include "session.hpp"

#include "base/config.hpp"
#include "base/log.hpp"

#include <sstream>

namespace web_socket
{

WebSocketSession::WebSocketSession(boost::asio::ip::tcp::socket&& socket, RequestCall callback)
  : _connected_endpoint{ socket.remote_endpoint() }
  , _web_socket{ std::move(socket) }
  , _request_call{ std::move(callback) }
{}


WebSocketSession::~WebSocketSession()
{
    disconnect();
}


void WebSocketSession::disconnect()
{
    LOG_DEBUG << "disconnecting:" << _connected_endpoint;

    boost::beast::error_code ec;
    _web_socket.close(boost::beast::websocket::close_code::service_restart, ec);

    if (ec) {
        LOG_ERROR << "closing failed: " << ec.message();
    }
}


void WebSocketSession::run()
{
    boost::beast::error_code ec;
    _web_socket.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

    _web_socket.set_option(
      boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::response_type& res) {
          res.set(boost::beast::http::field::server,
                  std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
      }));

    _web_socket.async_accept(boost::beast::bind_front_handler(&WebSocketSession::on_accept, shared_from_this()));
}


void WebSocketSession::on_accept(boost::beast::error_code ec)
{
    if (ec) {
        LOG_ERROR << "Accept error: " << ec.message();
        return;
    }
    do_read();
}


void WebSocketSession::do_read()
{
    _buffer.clear();
    _buffer.reserve(base::config::RPC_MESSAGE_BUFFER_SIZE);
    _web_socket.async_read(_buffer, boost::beast::bind_front_handler(&WebSocketSession::on_read, shared_from_this()));
}


void WebSocketSession::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec) {
        LOG_DEBUG << "Connection closed by reason:" << ec;
        disconnect();
        return;
    }

    base::Bytes received_data(bytes_transferred);
    std::memcpy(received_data.getData(), _buffer.data().data(), bytes_transferred);
    try {
        _request_call(base::parseJson(received_data.toString()),
                      [this](base::PropertyTree responce) { do_write(std::move(responce)); });
    }
    catch (const base::Error& error) {
        LOG_ERROR << "client json parsing fail: " << error;
    }

    do_read();
}


void WebSocketSession::do_write(base::PropertyTree responce)
{
    _web_socket.text(true);
    _web_socket.async_write(boost::asio::buffer(responce.toString()),
                            boost::beast::bind_front_handler(&WebSocketSession::on_write, shared_from_this()));
}


void WebSocketSession::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        LOG_ERROR << "Write error: " << ec.message();
        return;
    }
}

}