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
                                 ReceiveMessageCallback receiveData,
                                 CloseCallback socketClosed)
  : _resolver(boost::asio::make_strand(ioc))
  , _websocket(boost::asio::make_strand(ioc))
  , _ready{ false }
  , _timer(_websocket.get_executor(), (std::chrono::steady_clock::time_point::max)())
  , _state(EndPointState::NO_CONNECTION)
  , _receive_callback{ std::move(receiveData) }
  , _close_callback{ std::move(socketClosed) }
  , _last_given_query_id{ 0 }
{}


WebSocketClient::~WebSocketClient() noexcept
{
    disconnect();
}


bool WebSocketClient::connect(const std::string& host)
{
    if (_ready) {
        LOG_ERROR << "socket already connected to address[ip_v4]: " << _websocket.next_layer().remote_endpoint();
        return false;
    }

    boost::asio::ip::tcp::endpoint targetEndpoint;
    try {
        targetEndpoint = createEndpoint(host);
    }
    catch (const base::InvalidArgument& er) {
        LOG_ERROR << "target host ip decoding error: " << er.what();
        return _ready;
    }

    boost::system::error_code ec;
    auto const results = _resolver.resolve(targetEndpoint, ec);
    if (ec) {
        LOG_ERROR << "resolving error: " << ec.message();
        return _ready;
    }

    auto ep = boost::asio::connect(_websocket.next_layer(), results, ec);
    if (ec) {
        LOG_ERROR << "connection error: " << ec.message();
        return _ready;
    }

    _websocket.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));
    static const std::string userAgentName{ std::string(BOOST_BEAST_VERSION_STRING) +
                                            std::string(" websocket-client") };
    _websocket.set_option(
      boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type& req) {
          req.set(boost::beast::http::field::user_agent, userAgentName);
      }));

    auto resolvedHost = host + ':' + std::to_string(ep.port());
    _websocket.handshake(resolvedHost, "/", ec);
    if (ec) {
        LOG_ERROR << "handshake failed: " << ec.message();
        close(boost::beast::websocket::close_code::internal_error);
        return _ready;
    }

    doRead();

    _ready = true;
    _websocket.control_callback(
      std::bind(&WebSocketClient::frameControl, this, std::placeholders::_1, std::placeholders::_2));

    active();
    onTimer({});

    return _ready;
}


void WebSocketClient::onTimer(boost::system::error_code ec)
{
    if (ec && ec != boost::asio::error::operation_aborted) {
        LOG_ERROR << "connection error: " << ec.message();
        return;
    }

    if (_timer.expiry() <= std::chrono::steady_clock::now()) {
        if (_websocket.is_open() && _state == EndPointState::ALIVE) {
            _state = EndPointState::PING_SENDING;
            _timer.expires_after(std::chrono::seconds(15));

            _websocket.async_ping(
              {},
              boost::asio::bind_executor(_websocket.get_executor(),
                                         std::bind(&WebSocketClient::onPing, this, std::placeholders::_1)));
        }
        else {
            closeTimeout();
            _state = EndPointState::NO_CONNECTION;
            return;
        }
    }

    _timer.async_wait(boost::asio::bind_executor(_websocket.get_executor(),
                                                 std::bind(&WebSocketClient::onTimer, this, std::placeholders::_1)));
}

void WebSocketClient::active()
{
    _state = EndPointState::ALIVE;
    _timer.expires_after(std::chrono::seconds(15));
}


void WebSocketClient::onPing(boost::system::error_code ec)
{
    if (ec == boost::asio::error::operation_aborted) {
        LOG_ERROR << "connection error: " << ec.message();
        return;
    }

    if (ec) {
        LOG_ERROR << "ping failed: " << ec.message();
        return;
    }
    if (_state == EndPointState::PING_SENDING) {
        _state = EndPointState::PING_SEND;
    }
}


void WebSocketClient::frameControl(boost::beast::websocket::frame_type kind,
                                   [[maybe_unused]] boost::string_view payload) noexcept
{
    if (kind == boost::beast::websocket::frame_type::close) {
        disconnect();
    }
    else {
        active();
    }
}


void WebSocketClient::send(Command::Id commandId, base::json::Value&& args)
{
    auto command_type = serializeCommandType(commandId);
    auto command_name = serializeCommandName(commandId);

    auto query = base::json::Value::object();
    query["type"] = serializeCommandType(commandId);
    query["name"] = serializeCommandName(commandId);
    query["version"] = base::json::Value::number(base::config::PUBLIC_SERVICE_API_VERSION);
    query["id"] = base::json::Value::number(registerNewQuery(commandId));
    query["args"] = args;

    if (!_ready) {
        RAISE_ERROR(base::LogicError, "client is not ready");
    }

    auto output = query.serialize();
    boost::beast::error_code ec;
    _websocket.write(boost::asio::buffer(output), ec);
    if (ec) {
        LOG_ERROR << "write failed: " << ec.message();
        close(boost::beast::websocket::close_code::internal_error);
    }
}


bool WebSocketClient::ready()
{
    return _websocket.is_open();
}


void WebSocketClient::disconnect() noexcept
{
    close(boost::beast::websocket::close_code::normal);
}


void WebSocketClient::close(boost::beast::websocket::close_code reason) noexcept
{
    if (_websocket.is_open()) {
        boost::beast::error_code ec;

        _websocket.close(reason, ec);

        if (ec) {
            LOG_ERROR << "closing failed: " << ec.message();
            return;
        }

        _ready = false;

        if (_close_callback) {
            try {
                _close_callback();
            }
            catch (const std::exception& ex) {
                LOG_DEBUG << "exception at close callback " << ex.what();
            }
            catch (...) {
                LOG_DEBUG << "unexpected exception at close callback";
            }
        }
    }
}


void WebSocketClient::closeTimeout()
{
    boost::beast::error_code ec;
    _websocket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    _websocket.next_layer().close(ec);

    if (ec) {
        LOG_ERROR << "closing failed: " << ec.message();
        return;
    }

    _ready = false;

    if (_close_callback) {
        try {
            _close_callback();
        }
        catch (const std::exception& ex) {
            LOG_DEBUG << "exception at close callback " << ex.what();
        }
        catch (...) {
            LOG_DEBUG << "unexpected exception at close callback";
        }
    }
}


void WebSocketClient::doRead()
{
    _websocket.async_read(_read_buffer, boost::beast::bind_front_handler(&WebSocketClient::onRead, this));
}


void WebSocketClient::onRead(boost::beast::error_code ec, std::size_t bytesTransferred)
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

    base::Bytes receivedData(bytesTransferred);
    std::memcpy(receivedData.getData(), _read_buffer.data().data(), bytesTransferred);
    _read_buffer.clear();

    doRead();

    base::json::Value receivedQuery;
    try {
        receivedQuery = base::json::Value::parse(receivedData.toString());
    }
    catch (const std::exception& ex) {
        LOG_DEBUG << "parse query json error: " << ex.what();
        return;
    }
    catch (...) {
        LOG_DEBUG << "unexpected parse query json error.";
        return;
    }

    if (!receivedQuery.has_number_field("id")) {
        LOG_DEBUG << "Request json is not contain an uint \"id\" member";
        return;
    }
    auto id_json_value = receivedQuery["id"].as_number();
    if (!id_json_value.is_uint64()) {
        RAISE_ERROR(base::InvalidArgument, "Request \"id\" member is not an uint type");
    }
    QueryId query_id{ id_json_value.to_uint64() };

    if (!receivedQuery.has_string_field("status")) {
        LOG_DEBUG << "Request json is not contain an uint \"status\" member";
        return;
    }
    std::string status = receivedQuery["status"].as_string();

    if (!receivedQuery.has_string_field("type")) {
        LOG_DEBUG << "Request json is not contain an string \"type\" member";
        return;
    }
    std::string type = receivedQuery["type"].as_string();

    if (!receivedQuery.has_object_field("result")) {
        LOG_DEBUG << "Request json is not contain an object \"type\" member";
        return;
    }
    auto result_json_value = receivedQuery["result"];

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
        auto currentCommandId = Command::Id(command_id->second);
        if (!static_cast<bool>(currentCommandId & Command::Id(Command::Type::SUBSCRIBE))) {
            _current_queries.erase(command_id);
        }
        _receive_callback(currentCommandId, std::move(result_json_value));
    }
    catch (const base::Error& error) {
        LOG_ERROR << "receive callback execution error" << error.what();
    }
}


QueryId WebSocketClient::registerNewQuery(Command::Id commandId)
{
    auto currentId = ++_last_given_query_id;
    _current_queries.insert({ currentId, commandId });
    return currentId;
}

}
