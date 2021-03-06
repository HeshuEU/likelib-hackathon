#pragma once

#include "types.hpp"

#include "base/json.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>

namespace websocket
{

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient>
{

    using ReceiveMessageCallback = std::function<void(Command::Id, base::json::Value&&)>;
    using CloseCallback = std::function<void(void)>;

  public:
    explicit WebSocketClient(boost::asio::io_context& ioc,
                             ReceiveMessageCallback receiveData,
                             CloseCallback socketClosed = {});
    ~WebSocketClient() noexcept;

    bool connect(const std::string& host);
    void disconnect() noexcept;

    void send(Command::Id commandId, base::json::Value&& args);

    bool ready();

  private:
    boost::asio::ip::tcp::resolver _resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> _websocket;

    bool _ready;

    boost::beast::flat_buffer _read_buffer;

    ReceiveMessageCallback _receive_callback;
    CloseCallback _close_callback;

    QueryId _last_given_query_id;
    std::unordered_map<QueryId, Command::Id> _current_queries;

    void frameControl(boost::beast::websocket::frame_type kind, boost::string_view payload) noexcept;

    void close(boost::beast::websocket::close_code reason) noexcept;

    void doRead();
    void onRead(boost::beast::error_code ec, std::size_t bytesTransferred);

    QueryId registerNewQuery(Command::Id commandId);
};

}
