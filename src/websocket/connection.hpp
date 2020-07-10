#pragma once

#include "base/property_tree.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <memory>

namespace websocket
{

class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection>
{
    using ConnectionCloseCallback = std::function<void(void)>;
    using ProcessRequestCallback = std::function<void(base::PropertyTree)>;

  public:
    explicit WebSocketConnection(boost::asio::ip::tcp::socket&& socket,
                                 ProcessRequestCallback requestCallback,
                                 ConnectionCloseCallback closeCallback);
    ~WebSocketConnection() noexcept;

    void accept();
    void write(base::PropertyTree&& response);
    void close();

  private:
    boost::asio::ip::tcp::endpoint _connectedEndpoint;
    boost::beast::websocket::stream<boost::beast::tcp_stream> _websocket;
    bool _is_ready;

    boost::beast::flat_buffer _readBuffer;

    ProcessRequestCallback _process;
    ConnectionCloseCallback _closed;

    void onAccept(boost::beast::error_code ec);

    void doRead();
    void onRead(boost::beast::error_code ec, std::size_t bytesTransferred);

    void doClose() noexcept;
};

}