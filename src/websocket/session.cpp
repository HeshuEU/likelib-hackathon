#include "session.hpp"
#include "tools.hpp"

#include "base/config.hpp"
#include "base/log.hpp"

namespace
{

base::json::Value _makeAnswer(websocket::QueryId id, bool is_success, base::json::Value&& result)
{
    base::json::Value answer;
    answer["type"] = base::json::Value::string("answer");
    if (is_success) {
        answer["status"] = base::json::Value::string("success");
    }
    else {
        answer["status"] = base::json::Value::string("error");
    }
    answer["id"] = base::json::Value::number(id);
    answer["result"] = result;
    return answer;
}


base::json::Value makeAnswer(websocket::QueryId id, base::json::Value&& result)
{
    return _makeAnswer(id, true, std::move(result));
}


base::json::Value makeErrorAnswer(websocket::QueryId id, const std::string& message)
{
    base::json::Value result;
    result["error_message"] = base::json::Value::string(message);
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
    LOG_TRACE << "created session[" << _session_id << "]";
}


void WebSocketSession::sendResult(QueryId queryId, base::json::Value result)
{
    auto prepared_answer = makeAnswer(queryId, std::move(result));
    LOG_TRACE << "prepared success result at session[" << _session_id << "] by query[" << queryId << "]";
    std::lock_guard lock{ _connection_rw_mutex };
    return _send(std::move(prepared_answer));
}


void WebSocketSession::sendErrorMessage(QueryId queryId, const std::string& error_message)
{
    auto prepared_answer = makeErrorAnswer(queryId, error_message);
    LOG_TRACE << "prepared error result at session[" << _session_id << "] by query[" << queryId << "]";
    std::lock_guard lock{ _connection_rw_mutex };
    return _send(std::move(prepared_answer));
}


void WebSocketSession::_send(base::json::Value result)
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


void WebSocketSession::_onReceivedFromConnection(base::json::Value&& query)
{
    if (!query.has_number_field("id")) {
        return sendErrorMessage(0, "Request json is not contain number \"id\" member");
    }
    auto id_json_value = query["id"].as_number();
    if (!id_json_value.is_uint64()) {
        return sendErrorMessage(0, "Request \"id\" member is not an uint type");
    }
    QueryId query_id{ id_json_value.to_uint64() };

    if (!query.has_number_field("version")) {
        return sendErrorMessage(query_id, "Request json is not contain number \"version\" member");
    }
    auto version_json_value = query["version"].as_number();
    if (!version_json_value.is_uint64()) {
        return sendErrorMessage(query_id, "Request \"version\" member is not an uint type");
    }
    if (version_json_value.to_uint64() != base::config::PUBLIC_SERVICE_API_VERSION) {
        return sendErrorMessage(query_id,
                                std::string("Request API version mismatch. Need API version = ") +
                                  std::to_string(base::config::PUBLIC_SERVICE_API_VERSION));
    }

    if (!query.has_string_field("type")) {
        return sendErrorMessage(query_id, "Request json is not contain string \"type\" member");
    }
    auto command_type = deserializeCommandType(query["type"].as_string());

    if (!query.has_string_field("name")) {
        return sendErrorMessage(query_id, "Request json is not contain string \"name\" member");
    }
    auto command_name = deserializeCommandName(query["name"].as_string());

    if (!query.has_object_field("args")) {
        return sendErrorMessage(query_id, "Request json is not contain object \"args\" member");
    }
    auto args_json_value = query["args"];

    {
        std::lock_guard lock{ _connection_rw_mutex };
        if (_registeredQueryIds.contains(query_id)) {
            return sendErrorMessage(query_id, "Request id already exists in current session");
        }

        LOG_TRACE << "registered query at session[" << _session_id << "], with id[" << query_id << "]";
        _registeredQueryIds.insert(query_id);
    }

    try {
        _request_process(
          _session_id, query_id, Command::Id(command_type) | Command::Id(command_name), std::move(args_json_value));
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