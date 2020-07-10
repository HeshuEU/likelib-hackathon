#pragma once

#include "connection.hpp"
#include "types.hpp"

#include "base/bytes.hpp"
#include "base/property_tree.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <memory>
#include <mutex>

namespace websocket
{

class WebSocketSession
{
    using ProcessRequestCallback = std::function<void(SessionId, QueryId, Command::Id, base::PropertyTree&&)>;
    using SessionCloseCallback = std::function<void(SessionId)>;

  public:
    explicit WebSocketSession(boost::asio::ip::tcp::socket&& socket,
                              SessionId id,
                              ProcessRequestCallback processCallback,
                              SessionCloseCallback closeCallback);
    ~WebSocketSession() = default;

    void sendResult(QueryId queryId, base::PropertyTree&& result);
    void sendErrorMessage(QueryId queryId, const std::string& result);

  private:
    SessionId _sessionId;
    std::weak_ptr<WebSocketConnection> _connectionPointer;
    std::mutex _connectionMutex;
    bool _isReady;

    ProcessRequestCallback _requestProcess;
    SessionCloseCallback _closeSession;

    std::set<QueryId> _registeredQueryIds;

    void send(base::PropertyTree&& result);

    void onDataReceivedFromConnection(base::PropertyTree&& query);
    void onConnectionClosed();
};

}