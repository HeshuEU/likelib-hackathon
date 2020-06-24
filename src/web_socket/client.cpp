#include "client.hpp"
#include "tools.hpp"

#include "base/bytes.hpp"
#include "base/log.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <cstring>


namespace web_socket
{

WebSocketClient::WebSocketClient(boost::asio::io_context& ioc,
                                 ReceiveMessageCallback receive_callback,
                                 CloseCallback close_callback)
  : _resolver(boost::asio::make_strand(ioc))
  , _web_socket(boost::asio::make_strand(ioc))
  , _receive_callback{ std::move(receive_callback) }
  , _close_callback{ std::move(close_callback) }
{}


WebSocketClient::~WebSocketClient()
{
    disconnect();
}


bool WebSocketClient::connect(const std::string& host)
{
    boost::system::error_code ec;
    auto const results = _resolver.resolve(create_endpoint(host), ec);
    if (ec) {
        LOG_ERROR << "resolving error: " << ec.message();
        return false;
    }

    auto ep = boost::asio::connect(_web_socket.next_layer(), results, ec);
    if (ec) {
        LOG_ERROR << "connection error: " << ec.message();
        return false;
    }

    _web_socket.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));
    static const std::string user_agent_name{ std::string(BOOST_BEAST_VERSION_STRING) +
                                              std::string(" websocket-client") };
    _web_socket.set_option(
      boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type& req) {
          req.set(boost::beast::http::field::user_agent, user_agent_name);
      }));

    auto resolved_host = host + ':' + std::to_string(ep.port());
    _web_socket.handshake(resolved_host, "/", ec);
    if (ec) {
        LOG_ERROR << "handshake failed: " << ec.message();
        close(boost::beast::websocket::close_code::internal_error);
        return false;
    }

    do_read();

    _ready = true;
    _web_socket.control_callback(
      std::bind(&WebSocketClient::control_callback, this, std::placeholders::_1, std::placeholders::_2));
    return _ready;
}


void WebSocketClient::control_callback(boost::beast::websocket::frame_type kind,
                                       [[maybe_unused]] boost::string_view payload)
{
    if (kind == boost::beast::websocket::frame_type::close) {
        disconnect();
    }
}


bool WebSocketClient::is_connected()
{
    return _ready;
}


void WebSocketClient::send(const base::PropertyTree& query)
{
    _web_socket.async_write(boost::asio::buffer(query.toString()),
                            boost::beast::bind_front_handler(&WebSocketClient::on_write, this));
}


void WebSocketClient::disconnect()
{
    close(boost::beast::websocket::close_code::normal);
}


void WebSocketClient::close(boost::beast::websocket::close_code reason)
{
    if (_web_socket.is_open()) {
        boost::beast::error_code ec;
        _web_socket.close(reason, ec);

        if (ec) {
            LOG_ERROR << "closing failed: " << ec.message();
            return;
        }

        _ready = false;
        if (_close_callback) {
            _close_callback();
        }
    }
}


void WebSocketClient::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        LOG_ERROR << "write failed: " << ec.message();
        close(boost::beast::websocket::close_code::internal_error);
    }
}


void WebSocketClient::do_read()
{
    _web_socket.async_read(_read_buffer, boost::beast::bind_front_handler(&WebSocketClient::on_read, this));
}


void WebSocketClient::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec == boost::beast::websocket::error::closed) {
        LOG_DEBUG << "Connection closed";
        close(boost::beast::websocket::close_code::bad_payload);
        return;
    }

    if (ec) {
        LOG_WARNING << "read failed: " << ec.message();
        return;
    }

    base::Bytes received_data(bytes_transferred);
    std::memcpy(received_data.getData(), _read_buffer.data().data(), bytes_transferred);

    do_read();

    base::PropertyTree received_query;
    try {
        received_query = base::parseJson(received_data.toString());
    }
    catch (const base::Error& error) {
        LOG_ERROR << "client json parsing fail: " << error;
    }

    _receive_callback(std::move(received_query));
}

}
