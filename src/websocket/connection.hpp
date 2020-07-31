#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "base/json.hpp"

#include <functional>
#include <memory>

namespace websocket
{

class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection>
{
    using ConnectionCloseCallback = std::function<void(void)>;
    using ProcessRequestCallback = std::function<void(base::json::Value&&)>;

  public:
    explicit WebSocketConnection(boost::asio::ip::tcp::socket&& socket,
                                 ProcessRequestCallback request_callback,
                                 ConnectionCloseCallback close_callback);
    ~WebSocketConnection() noexcept;

    void accept();
    void write(base::json::Value response);
    void close();

  private:
    boost::asio::ip::tcp::endpoint _connected_endpoint;
    boost::beast::websocket::stream<boost::beast::tcp_stream> _websocket;
    bool _is_ready;

    boost::beast::flat_buffer _read_buffer;

    ProcessRequestCallback _process_callback;
    ConnectionCloseCallback _close_callback;

    void onAccept(boost::beast::error_code ec);

    void doRead();
    void onRead(boost::beast::error_code ec, std::size_t bytes_transferred);

    void doClose() noexcept;
};

}