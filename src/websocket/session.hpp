#pragma once

#include "connection.hpp"
#include "types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <memory>
#include <mutex>

namespace websocket
{

class WebSocketSession
{
    using ProcessRequestCallback = std::function<void(SessionId, QueryId, Command::Id, base::json::Value&&)>;
    using SessionCloseCallback = std::function<void(SessionId)>;

  public:
    explicit WebSocketSession(boost::asio::ip::tcp::socket&& socket,
                              SessionId id,
                              ProcessRequestCallback process_callback,
                              SessionCloseCallback close_callback);
    ~WebSocketSession() = default;

    void sendResult(QueryId queryId, base::json::Value result);
    void sendErrorMessage(QueryId queryId, const std::string& error_message);

  private:
    SessionId _session_id;
    std::weak_ptr<WebSocketConnection> _connection_pointer;
    std::mutex _connection_rw_mutex;
    bool _is_ready;

    ProcessRequestCallback _request_process;
    SessionCloseCallback _close_session_callback;

    std::set<QueryId> _registeredQueryIds;

    void _send(base::json::Value result);

    void _onReceivedFromConnection(base::json::Value&& query);
    void _onConnectionClosed();
};

}