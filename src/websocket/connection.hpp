#pragma once

#include "base/property_tree.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <memory>


namespace websocket
{

using ConnectionCloseCallback = std::function<void(void)>;
using ConnectionReceivedCallback = std::function<void(base::PropertyTree)>;


class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection>
{
  public:
    explicit WebSocketConnection(boost::asio::ip::tcp::socket&& socket,
                                 ConnectionReceivedCallback request_callback,
                                 ConnectionCloseCallback close_callback);
    ~WebSocketConnection();

    void accept();
    void write(base::PropertyTree&& response);
    void close();

  private:
    boost::asio::ip::tcp::endpoint _connected_endpoint;
    boost::beast::websocket::stream<boost::beast::tcp_stream> _web_socket;
    bool _is_ready;

    boost::beast::flat_buffer _read_buffer;

    ConnectionReceivedCallback _request_callback;
    ConnectionCloseCallback _close_callback;

    void on_accept(boost::beast::error_code ec);

    void do_read();
    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);

    void do_close();
};

}