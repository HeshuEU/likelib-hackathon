#include "client.hpp"
#include "tools.hpp"

#include "base/bytes.hpp"
#include "base/log.hpp"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

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
  , _receiveCallback{ std::move(receiveData) }
  , _closeCallback{ std::move(socketClosed) }
  , _lastGivenQueryId{ 0 }
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

    return _ready;
}


void WebSocketClient::frameControl(boost::beast::websocket::frame_type kind,
                                   [[maybe_unused]] boost::string_view payload) noexcept
{
    if (kind == boost::beast::websocket::frame_type::close) {
        disconnect();
    }
}


void WebSocketClient::send(Command::Id commandId, rapidjson::Document&& args)
{
    auto command_type = serializeCommandType(commandId);
    auto command_name = serializeCommandName(commandId);

    rapidjson::Document query(rapidjson::kObjectType);
    query.AddMember("type", rapidjson::StringRef(command_type.c_str()), query.GetAllocator());
    query.AddMember("name", rapidjson::StringRef(command_name.c_str()), query.GetAllocator());
    query.AddMember("version", rapidjson::Value(base::config::PUBLIC_SERVICE_API_VERSION), query.GetAllocator());
    query.AddMember("id", rapidjson::Value(registerNewQuery(commandId)), query.GetAllocator());
    rapidjson::Value args_value(rapidjson::kObjectType);
    args_value = args.GetObject();
    query.AddMember("args", args_value, query.GetAllocator());

    if (!_ready) {
        RAISE_ERROR(base::LogicError, "client is not ready");
    }

    std::string output;
    {
        MyOStreamWrapper os;
        rapidjson::Writer<MyOStreamWrapper> writer(os);
        query.Accept(writer);
        output = os.toString();
    }

    boost::beast::error_code ec;
    _websocket.write(boost::asio::buffer(output), ec);
    if (ec) {
        LOG_ERROR << "write failed: " << ec.message();
        close(boost::beast::websocket::close_code::internal_error);
    }
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
        if (_closeCallback) {
            try {
                _closeCallback();
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


void WebSocketClient::doRead()
{
    _websocket.async_read(_readBuffer, boost::beast::bind_front_handler(&WebSocketClient::onRead, this));
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
    std::memcpy(receivedData.getData(), _readBuffer.data().data(), bytesTransferred);
    _readBuffer.clear();

    doRead();

    rapidjson::Document receivedQuery;
    receivedQuery.Parse(receivedData.toString().c_str());
    if (receivedQuery.HasParseError()) {
        LOG_DEBUG << "parse query json error.";
        return;
    }

    if (!receivedQuery.HasMember("id")) {
        LOG_DEBUG << "Request json is not contain \"id\" member";
        return;
    }
    auto id_json_value = receivedQuery.FindMember("id");
    if (!(id_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "Request \"id\" member is not an uint type");
    }
    QueryId queryId = id_json_value->value.GetUint64();

    if (!receivedQuery.HasMember("status")) {
        LOG_DEBUG << "Request json is not contain \"status\" member";
        return;
    }
    auto status_json_value = receivedQuery.FindMember("status");
    if (!(status_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Request \"status\" member is not an string type");
    }
    std::string status = status_json_value->value.GetString();

    if (!receivedQuery.HasMember("type")) {
        LOG_DEBUG << "Request json is not contain \"type\" member";
        return;
    }
    auto type_json_value = receivedQuery.FindMember("type");
    if (!(type_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Request \"type\" member is not an string type");
    }
    std::string type = type_json_value->value.GetString();

    if (!receivedQuery.HasMember("type")) {
        LOG_DEBUG << "Request json is not contain \"type\" member";
        return;
    }
    auto result_json_value = receivedQuery.FindMember("result");
    if (!(result_json_value->value.IsObject())) {
        RAISE_ERROR(base::InvalidArgument, "Request \"result\" member is not an object type");
    }

    if (type != "answer") {
        LOG_DEBUG << "wrong answer type";
        return;
    }

    try {
        auto commandId = _currentQueries.find(queryId);
        if (commandId == _currentQueries.end()) {
            LOG_DEBUG << "received unregistered answer id";
            return;
        }
        auto currentCommandId = Command::Id(commandId->second);
        if (!static_cast<bool>(currentCommandId & Command::Id(Command::Type::SUBSCRIBE))) {
            _currentQueries.erase(commandId);
        }
        _receiveCallback(currentCommandId, std::move(result_json_value->value.GetObject()));
    }
    catch (const base::Error& error) {
        LOG_ERROR << "receive callback execution error" << error.what();
    }
}


QueryId WebSocketClient::registerNewQuery(Command::Id commandId)
{
    auto currentId = ++_lastGivenQueryId;
    _currentQueries.insert({ currentId, commandId });
    return currentId;
}

}
