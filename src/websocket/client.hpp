#pragma once

#include "types.hpp"

#include <base/property_tree.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>


namespace websocket
{

class WebSocketClient
{
    using ReceiveMessageCallback = std::function<void(Command::Id, base::PropertyTree)>;
    using CloseCallback = std::function<void(void)>;

  public:
    explicit WebSocketClient(boost::asio::io_context& ioc,
                             ReceiveMessageCallback receiveData,
                             CloseCallback socketClosed = {});
    ~WebSocketClient() noexcept;

    bool connect(const std::string& host);
    void disconnect() noexcept;

    void send(Command::Id commandId, const base::PropertyTree& args);

  private:
    boost::asio::ip::tcp::resolver _resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> _websocket;
    bool _ready;

    boost::beast::flat_buffer _readBuffer;

    ReceiveMessageCallback _receiveCallback;
    CloseCallback _closeCallback;

    QueryId _lastGivenQueryId;
    std::unordered_map<QueryId, Command::Id> _currentQueries;

    void frameControl(boost::beast::websocket::frame_type kind, boost::string_view payload) noexcept;

    void close(boost::beast::websocket::close_code reason) noexcept;

    void doRead();
    void onRead(boost::beast::error_code ec, std::size_t bytesTransferred);

    QueryId registerNewQuery(Command::Id commandId);
};

}
