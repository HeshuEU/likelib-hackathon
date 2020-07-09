#include "session.hpp"
#include "tools.hpp"

#include "base/config.hpp"
#include "base/log.hpp"


namespace
{

template<typename ResultType>
base::PropertyTree makeAnswer(websocket::QueryId id, bool is_success, ResultType result)
{
    base::PropertyTree answer;
    answer.add("type", "answer");
    if (is_success) {
        answer.add("status", "success");
    }
    else {
        answer.add("status", "error");
    }
    answer.add("id", id);
    answer.add("result", result);
    return answer;
}


base::PropertyTree makeErrorAnswer(websocket::QueryId id, const std::string& message)
{
    return makeAnswer(id, false, message);
}

}

namespace websocket
{

WebSocketSession::WebSocketSession(boost::asio::ip::tcp::socket&& socket,
                                   SessionId id,
                                   SessionReceivedDataCallback received_callback,
                                   SessionCloseCallback close_callback)
  : _session_id{ id }
  , _is_ready{ false }
  , _request_callback{ std::move(received_callback) }
  , _close_callback{ std::move(close_callback) }
{
    ASSERT(_request_callback);
    ASSERT(_close_callback);

    auto connection = std::make_shared<WebSocketConnection>(
      std::move(socket),
      std::bind(&WebSocketSession::on_connection_received, this, std::placeholders::_1),
      std::bind(&WebSocketSession::on_connection_close, this));
    connection->accept();
    _connection_ptr = connection;
    _is_ready = true;
}


void WebSocketSession::on_connection_received(base::PropertyTree&& query)
{
    QueryId query_id{ 0 };
    Command::Name command_name;
    Command::Type command_type;
    base::PropertyTree command_args;
    try {
        query_id = query.get<QueryId>("id");
        command_name = deserializeCommandName(query.get<std::string>("name"));
        command_type = deserializeCommandType(query.get<std::string>("type"));
        command_args = query.getSubTree("args");
    }
    catch (const base::Error& e) {
        LOG_ERROR << "deserialization error" << e.what();
        return sendErrorMessage(query_id, "deserialization error");
    }

    auto command_id = Command::Id(command_type) | Command::Id(command_name);

    if (query.get<std::uint32_t>("api") != base::config::RPC_PUBLIC_API_VERSION) {
        LOG_DEBUG << "api version mismatch";
        return sendErrorMessage(query_id, "api version mismatch");
    }

    if (_registered_query_id_set.contains(query_id)) {
        LOG_DEBUG << "query id already exists in session";
        return sendErrorMessage(query_id, "query id already exists in session");
    }

    LOG_DEBUG << "registered query at session[" << _session_id << "]";

    _registered_query_id_set.insert(query_id);
    try {
        _request_callback(_session_id, query_id, command_id, std::move(command_args));
    }
    catch (const base::Error& error) {
        LOG_ERROR << "request callback execution error" << error.what();
    }
}


void WebSocketSession::on_connection_close()
{
    if (_is_ready) {
        _is_ready = false;
        LOG_DEBUG << "called close callback at session[" << _session_id << "]";
        _close_callback(_session_id);
    }
}


void WebSocketSession::sendResult(QueryId query_id, base::PropertyTree&& result)
{
    LOG_DEBUG << "sending success result at session[" << _session_id << "] by query[" << query_id << "]";
    return send(makeAnswer(query_id, true, result));
}


void WebSocketSession::sendErrorMessage(QueryId query_id, const std::string& result)
{
    LOG_DEBUG << "sending error result at session[" << _session_id << "] by query[" << query_id << "]";
    return send(makeErrorAnswer(query_id, result));
}


void WebSocketSession::send(base::PropertyTree&& result)
{
    if (!_is_ready) {
        LOG_DEBUG << "send broken by cause session is not ready";
        return;
    }

    if (auto spt = _connection_ptr.lock()) {
        spt->write(std::move(result));
    }
    else {
        on_connection_close();
    }
}

}