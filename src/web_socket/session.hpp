#pragma once

#include "base/bytes.hpp"
#include "base/property_tree.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <functional>
#include <memory>

namespace web_socket
{

using ResponceCall = std::function<void(base::PropertyTree)>;

using RequestCall = std::function<void(base::PropertyTree, ResponceCall)>;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
  public:
    explicit WebSocketSession(boost::asio::ip::tcp::socket&& socket, RequestCall call);

    ~WebSocketSession();

    void run();

  private:
    boost::asio::ip::tcp::endpoint _connected_endpoint;
    boost::beast::websocket::stream<boost::beast::tcp_stream> _web_socket;
    boost::beast::flat_buffer _buffer;
    RequestCall _request_call;

    void disconnect();

    void on_accept(boost::beast::error_code ec);

    void do_read();
    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

    void do_write(base::PropertyTree responce);
    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);
};

}