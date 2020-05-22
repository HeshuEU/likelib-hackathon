#pragma once

#include "base/bytes.hpp"
#include "net/endpoint.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

namespace net
{

class Connection : public std::enable_shared_from_this<Connection>
{
  public:
    //====================
    using ReceiveHandler = std::function<void(const base::Bytes&)>;
    using SendHandler = std::function<void()>;
    using CloseHandler = std::function<void()>;
    //====================
    Connection(boost::asio::io_context& io_context,
               boost::asio::ip::tcp::socket&& socket,
               CloseHandler close_handler = {});

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection& other) = delete;

    /*
     * stepped on this error 2 times: binded handler expects old this value.
     * but after move, if moved object is deleted, handler is called on deleted object
     * and segfault is caught.
     */
    Connection(Connection&&) = delete;
    Connection& operator=(Connection&&) = delete;

    ~Connection();
    //====================
    void close();
    bool isClosed() const noexcept;
    void setCloseHandler(CloseHandler handler);
    //====================
    void send(base::Bytes data);
    void send(base::Bytes data, SendHandler send_handler);
    void receive(std::size_t bytes_to_receive, ReceiveHandler receive_handler);
    //====================
    const Endpoint& getEndpoint() const;
    //====================
  private:
    //====================
    boost::asio::io_context& _io_context;
    boost::asio::ip::tcp::socket _socket;

    std::mutex _close_handler_mutex;
    CloseHandler _close_handler;

    std::unique_ptr<Endpoint> _connect_endpoint;

    std::atomic<bool> _is_closed{ false };
    //====================
    base::Bytes _read_buffer;
    //====================
    std::queue<std::pair<base::Bytes, SendHandler>> _pending_send_messages;
    std::recursive_mutex _pending_send_messages_mutex; // TODO: check if this is an overkill
    void sendPendingMessages();
    //====================
};


} // namespace net