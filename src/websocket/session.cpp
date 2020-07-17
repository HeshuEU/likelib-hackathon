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
                                   ProcessRequestCallback processCallback,
                                   SessionCloseCallback closeCallback)
  : _sessionId{ id }
  , _isReady{ false }
  , _requestProcess{ std::move(processCallback) }
  , _closeSession{ std::move(closeCallback) }
{
    ASSERT(_requestProcess);
    ASSERT(_closeSession);

    auto connection = std::make_shared<WebSocketConnection>(
      std::move(socket),
      std::bind(&WebSocketSession::onDataReceivedFromConnection, this, std::placeholders::_1),
      std::bind(&WebSocketSession::onConnectionClosed, this));
    connection->accept();
    _connectionPointer = connection;
    _isReady = true;
}


void WebSocketSession::sendResult(QueryId queryId, base::PropertyTree&& result)
{
    std::lock_guard lock{ _connectionMutex };
    LOG_DEBUG << "try to send success result at session[" << _sessionId << "] by query[" << queryId << "]";
    return send(makeAnswer(queryId, true, result));
}


void WebSocketSession::sendErrorMessage(QueryId queryId, const std::string& result)
{
    std::lock_guard lock{ _connectionMutex };
    LOG_DEBUG << "try to send error result at session[" << _sessionId << "] by query[" << queryId << "]";
    return send(makeErrorAnswer(queryId, result));
}


void WebSocketSession::send(base::PropertyTree&& result)
{
    if (!_isReady) {
        LOG_DEBUG << "send broken by cause session is not ready";
        return;
    }

    if (auto spt = _connectionPointer.lock()) {
        spt->write(std::move(result));
    }
    else {
        onConnectionClosed();
    }
}


void WebSocketSession::onDataReceivedFromConnection(base::PropertyTree&& query)
{
    QueryId queryId{ 0 };
    Command::Name commandName;
    Command::Type commandType;
    base::PropertyTree commandArgs;

    {
        std::lock_guard lock{ _connectionMutex };

        try {
            queryId = query.get<QueryId>("id");
            commandName = deserializeCommandName(query.get<std::string>("name"));
            commandType = deserializeCommandType(query.get<std::string>("type"));
            commandArgs = query.getSubTree("args");
        }
        catch (const base::Error& e) {
            LOG_ERROR << "deserialization error" << e.what();
            return send(makeErrorAnswer(queryId, "deserialization error"));
        }
        catch (const std::exception& e) {
            LOG_ERROR << "deserialization error" << e.what();
            return send(makeErrorAnswer(queryId, "deserialization error"));
        }
        catch (...) {
            LOG_ERROR << "unexpected deserialization error";
            return send(makeErrorAnswer(queryId, "deserialization error"));
        }

        if (query.get<std::uint32_t>("api") != base::config::RPC_PUBLIC_API_VERSION) {
            LOG_DEBUG << "api version mismatch";
            return send(makeErrorAnswer(queryId, "api version mismatch"));
        }

        if (_registeredQueryIds.contains(queryId)) {
            LOG_DEBUG << "query id already exists in session";
            return send(makeErrorAnswer(queryId, "query id already exists in session"));
        }

        LOG_DEBUG << "registered query at session[" << _sessionId << "], with id[" << queryId << "]";
        _registeredQueryIds.insert(queryId);
    }

    try {
        _requestProcess(
          _sessionId, queryId, Command::Id(commandType) | Command::Id(commandName), std::move(commandArgs));
    }
    catch (const base::Error& error) {
        LOG_ERROR << "request callback execution error" << error.what();
    }
}


void WebSocketSession::onConnectionClosed()
{
    LOG_DEBUG << "called close callback at session[" << _sessionId << "]";
    std::lock_guard lock{ _connectionMutex };
    if (_isReady) {
        _isReady = false;
        _closeSession(_sessionId);
    }
}

}