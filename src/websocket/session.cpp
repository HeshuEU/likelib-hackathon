#include "session.hpp"
#include "tools.hpp"

#include "base/config.hpp"
#include "base/log.hpp"

namespace
{

rapidjson::Document _makeAnswer(websocket::QueryId id, bool is_success, rapidjson::Document result)
{
    rapidjson::Document answer;
    answer.AddMember("type", rapidjson::Value("answer"), answer.GetAllocator());
    if (is_success) {
        answer.AddMember("status", rapidjson::Value("success"), answer.GetAllocator());
    }
    else {
        answer.AddMember("status", rapidjson::Value("error"), answer.GetAllocator());
    }
    answer.AddMember("id", rapidjson::Value(id), answer.GetAllocator());
    answer.AddMember("result", result, answer.GetAllocator());
    return answer;
}


rapidjson::Document makeAnswer(websocket::QueryId id, rapidjson::Document result)
{
    return _makeAnswer(id, true, std::move(result));
}


rapidjson::Document makeErrorAnswer(websocket::QueryId id, const std::string& message)
{
    rapidjson::Document result;
    result.AddMember("error_message", rapidjson::Value(rapidjson::StringRef(message.c_str())), result.GetAllocator());

    return _makeAnswer(id, false, std::move(result));
}

}

namespace websocket
{

WebSocketSession::WebSocketSession(boost::asio::ip::tcp::socket&& socket,
                                   SessionId id,
                                   ProcessRequestCallback process_callback,
                                   SessionCloseCallback close_callback)
  : _session_id{ id }
  , _is_ready{ false }
  , _request_process{ std::move(process_callback) }
  , _close_session_callback{ std::move(close_callback) }
{
    ASSERT(_request_process);
    ASSERT(_close_session_callback);

    auto connection = std::make_shared<WebSocketConnection>(
      std::move(socket),
      std::bind(&WebSocketSession::_onReceivedFromConnection, this, std::placeholders::_1),
      std::bind(&WebSocketSession::_onConnectionClosed, this));
    connection->accept();
    _connection_pointer = connection;
    _is_ready = true;
}


void WebSocketSession::sendResult(QueryId queryId, rapidjson::Document result)
{
    auto prepared_answer = makeAnswer(queryId, std::move(result));
    LOG_TRACE << "prepared success result at session[" << _session_id << "] by query[" << queryId << "]";
    std::lock_guard lock{ _connection_rw_mutex };
    return _send(std::move(prepared_answer));
}


void WebSocketSession::sendErrorMessage(QueryId queryId, const std::string& result)
{
    auto prepared_answer = makeErrorAnswer(queryId, result);
    LOG_TRACE << "prepared error result at session[" << _session_id << "] by query[" << queryId << "]";
    std::lock_guard lock{ _connection_rw_mutex };
    return _send(std::move(prepared_answer));
}


void WebSocketSession::_send(rapidjson::Document&& result)
{
    LOG_TRACE << "try to send result at session[" << _session_id << "]";

    if (!_is_ready) {
        LOG_DEBUG << "send broken by cause: session[" << _session_id << "] is not ready";
        return;
    }

    if (auto spt = _connection_pointer.lock()) {
        spt->write(std::move(result));
    }
    else {
        _onConnectionClosed();
    }
}


void WebSocketSession::_onReceivedFromConnection(rapidjson::Document&& query)
{
    if (!query.HasMember("id")) {
        return sendErrorMessage(0, "Request json is not contain \"id\" member");
    }
    auto id_json_value = query.FindMember("id");
    if (!(id_json_value->value.IsUint64())) {
        return sendErrorMessage(0, "Request \"id\" member is not an uint type");
    }
    QueryId query_id{ id_json_value->value.GetUint64() };

    if (!query.HasMember("version")) {
        return sendErrorMessage(query_id, "Request json is not contain \"version\" member");
    }
    auto version_json_value = query.FindMember("version");
    if (!(version_json_value->value.IsUint64())) {
        return sendErrorMessage(query_id, "Request \"version\" member is not an uint type");
    }
    auto version = version_json_value->value.GetUint64();
    if (version != base::config::PUBLIC_SERVICE_API_VERSION) {
        return sendErrorMessage(query_id,
                                std::string("Request API version mismatch. Need API version = ") +
                                  std::to_string(base::config::PUBLIC_SERVICE_API_VERSION));
    }

    if (!query.HasMember("type")) {
        return sendErrorMessage(query_id, "Request json is not contain \"type\" member");
    }
    auto type_json_value = query.FindMember("type");
    if (!(type_json_value->value.IsString())) {
        return sendErrorMessage(query_id, "Request \"type\" member is not a string type");
    }
    auto command_type = deserializeCommandType(type_json_value->value.GetString());

    if (!query.HasMember("name")) {
        return sendErrorMessage(query_id, "Request json is not contain \"name\" member");
    }
    auto name_json_value = query.FindMember("name");
    if (!(name_json_value->value.IsString())) {
        return sendErrorMessage(query_id, "Request \"name\" member is not a string type");
    }
    auto command_name = deserializeCommandName(name_json_value->value.GetString());

    if (!query.HasMember("args")) {
        return sendErrorMessage(query_id, "Request json is not contain \"args\" member");
    }
    auto args_json_value = query.FindMember("args");
    if (!(args_json_value->value.IsObject())) {
        return sendErrorMessage(query_id, "Request \"args\" member is not a object type");
    }

    {
        std::lock_guard lock{ _connection_rw_mutex };
        if (_registeredQueryIds.contains(query_id)) {
            return sendErrorMessage(query_id, "Request id already exists in current session");
        }

        LOG_TRACE << "registered query at session[" << _session_id << "], with id[" << query_id << "]";
        _registeredQueryIds.insert(query_id);
    }

    rapidjson::Document command_args;
    command_args.CopyFrom(args_json_value->value.Move(), command_args.GetAllocator());

    try {
        _request_process(
          _session_id, query_id, Command::Id(command_type) | Command::Id(command_name), std::move(command_args));
    }
    catch (const base::Error& error) {
        LOG_ERROR << "request callback execution error" << error.what();
        return sendErrorMessage(query_id, std::string("Unexpected process error: ") + error.what());
    }
}


void WebSocketSession::_onConnectionClosed()
{
    LOG_TRACE << "called close callback at session[" << _session_id << "]";
    {
        std::lock_guard lock{ _connection_rw_mutex };
        if (_is_ready) {
            _is_ready = false;
        }
        else {
            return;
        }
    }

    try {
        _close_session_callback(_session_id);
    }
    catch (...) {
        LOG_ERROR << "unexpected error at closing callback for session " << _session_id;
    }
}

}