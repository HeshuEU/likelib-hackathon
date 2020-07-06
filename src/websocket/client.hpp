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
  public:
    using ReceiveMessageCallback = std::function<void(Command::Id, base::PropertyTree)>;
    using CloseCallback = std::function<void(void)>;

    explicit WebSocketClient(boost::asio::io_context& ioc,
                             ReceiveMessageCallback receive_callback,
                             CloseCallback close_callback = {});
    ~WebSocketClient();

    bool connect(const std::string& host);
    void disconnect();

    void send(Command::Id command_id, const base::PropertyTree& args);

  private:
    boost::asio::ip::tcp::resolver _resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> _web_socket;
    bool _ready;

    boost::beast::flat_buffer _read_buffer;

    ReceiveMessageCallback _receive_callback;
    CloseCallback _close_callback;

    QueryId _last_query_id;
    std::unordered_map<QueryId, Command::Id> _current_queries;

    void frame_control_callback(boost::beast::websocket::frame_type kind, boost::string_view payload);

    void close(boost::beast::websocket::close_code reason);

    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);

    void do_read();
    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

    QueryId registerNewQuery(Command::Id command_id);
};

}
