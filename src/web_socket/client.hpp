#pragma once

#include <base/property_tree.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>


namespace web_socket
{

using ReceiveMessageCallback = std::function<void(base::PropertyTree)>;
using CloseCallback = std::function<void(void)>;


class WebSocketClient
{
  public:
    explicit WebSocketClient(boost::asio::io_context& ioc,
                             ReceiveMessageCallback receive_callback,
                             CloseCallback close_callback = {});
    ~WebSocketClient();

    bool connect(const std::string& host);
    void disconnect();
    bool is_connected();

    void send(const base::PropertyTree& args);

  private:
    boost::asio::ip::tcp::resolver _resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> _web_socket;
    bool _ready{ false };

    boost::beast::flat_buffer _read_buffer;

    ReceiveMessageCallback _receive_callback;
    CloseCallback _close_callback;

    void control_callback(boost::beast::websocket::frame_type kind, boost::string_view payload);

    void close(boost::beast::websocket::close_code reason);

    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);

    void do_read();
    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
};

}
