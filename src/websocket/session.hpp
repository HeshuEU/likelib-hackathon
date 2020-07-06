#pragma once

#include "connection.hpp"
#include "types.hpp"

#include "base/bytes.hpp"
#include "base/property_tree.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <memory>

namespace websocket
{

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
  public:
    using SessionReceivedDataCallback = std::function<void(SessionId, QueryId, Command::Id, base::PropertyTree&&)>;
    using SessionCloseCallback = std::function<void(SessionId)>;

    explicit WebSocketSession(boost::asio::ip::tcp::socket&& socket,
                              SessionId id,
                              SessionReceivedDataCallback received_callback,
                              SessionCloseCallback close_callback);
    ~WebSocketSession() = default;

    void sendResult(QueryId query_id, base::PropertyTree&& result);
    void sendErrorMessage(QueryId query_id, const std::string& result);

  private:
    SessionId _session_id;
    std::weak_ptr<WebSocketConnection> _connection_ptr;
    bool _is_ready;

    SessionReceivedDataCallback _request_callback;
    SessionCloseCallback _close_callback;

    std::set<QueryId> _registered_query_id_set;

    void send(base::PropertyTree&& result);

    void on_connection_received(base::PropertyTree&& query);
    void on_connection_close();
};

}