#include "client.hpp"
#include "tools.hpp"

#include "base/bytes.hpp"
#include "base/log.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <cstring>


namespace websocket
{

WebSocketClient::WebSocketClient(boost::asio::io_context& ioc,
                                 ReceiveMessageCallback receive_callback,
                                 CloseCallback close_callback)
  : _resolver(boost::asio::make_strand(ioc))
  , _web_socket(boost::asio::make_strand(ioc))
  , _ready{ false }
  , _receive_callback{ std::move(receive_callback) }
  , _close_callback{ std::move(close_callback) }
  , _last_query_id{ 0 }
{}


WebSocketClient::~WebSocketClient()
{
    disconnect();
}


bool WebSocketClient::connect(const std::string& host)
{
    if (_ready) {
        LOG_ERROR << "socket already connected to address[ip_v4]: " << _web_socket.next_layer().remote_endpoint();
        return false;
    }

    boost::asio::ip::tcp::endpoint target_endpoint;
    try {
        target_endpoint = create_endpoint(host);
    }
    catch (const base::InvalidArgument& er) {
        LOG_ERROR << "target host ip decoding error: " << er.what();
        return _ready;
    }

    boost::system::error_code ec;
    auto const results = _resolver.resolve(target_endpoint, ec);
    if (ec) {
        LOG_ERROR << "resolving error: " << ec.message();
        return _ready;
    }

    auto ep = boost::asio::connect(_web_socket.next_layer(), results, ec);
    if (ec) {
        LOG_ERROR << "connection error: " << ec.message();
        return _ready;
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
        return _ready;
    }

    do_read();

    _ready = true;
    _web_socket.control_callback(
      std::bind(&WebSocketClient::frame_control_callback, this, std::placeholders::_1, std::placeholders::_2));
    return _ready;
}


void WebSocketClient::frame_control_callback(boost::beast::websocket::frame_type kind,
                                             [[maybe_unused]] boost::string_view payload)
{
    if (kind == boost::beast::websocket::frame_type::close) {
        disconnect();
    }
}


void WebSocketClient::send(Command::Id command_id, const base::PropertyTree& args)
{
    if (!_ready) {
        RAISE_ERROR(base::LogicError, "client is not ready");
    }
    base::PropertyTree query;
    query.add("type", serializeCommandType(command_id));
    query.add("name", serializeCommandName(command_id));
    query.add("api", base::config::RPC_PUBLIC_API_VERSION);
    query.add("id", registerNewQuery(command_id));
    query.add("args", args);

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
        LOG_ERROR << "client json parsing fail: " << error.what();
    }

    QueryId query_id{ 0 };
    std::string type;
    std::string status;
    base::PropertyTree command_args;
    try {
        query_id = received_query.get<QueryId>("id");
        status = received_query.get<std::string>("status");
        type = received_query.get<std::string>("type");
        command_args = received_query.getSubTree("result");
    }
    catch (const base::Error& e) {
        LOG_ERROR << "deserialization error" << e.what();
        return;
    }

    if (type != "answer") {
        LOG_DEBUG << "wrong answer type";
        return;
    }

    try {
        auto command_id = _current_queries.find(query_id);
        if (command_id == _current_queries.end()) {
            LOG_DEBUG << "received unregistered answer id";
            return;
        }
        auto current_command_id = Command::Id(command_id->second);
        if (!static_cast<bool>(current_command_id & Command::Id(Command::Type::SUBSCRIBE))) {
            _current_queries.erase(command_id);
        }
        _receive_callback(current_command_id, std::move(command_args));
    }
    catch (const base::Error& error) {
        LOG_ERROR << "receive callback execution error" << error.what();
    }
}


QueryId WebSocketClient::registerNewQuery(Command::Id command_id)
{
    auto current_id = ++_last_query_id;
    _current_queries.insert({ current_id, command_id });
    return current_id;
}

}
