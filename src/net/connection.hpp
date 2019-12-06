#pragma once

#include "base/bytes.hpp"
#include "net/endpoint.hpp"
#include "net/packet.hpp"

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
    enum class Status
    {
        ACCEPTED,
        CONNECTED,

        HANDSHAKE_1,
        HANDSHAKE_2,

        WAITING_FOR_PONG
    };

    using ReceiveHandler = std::function<void(Packet&&)>;
    //====================
    Connection(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket&& socket);

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
    void startSession(ReceiveHandler receive_handler);

    void close();
    bool isClosed() const noexcept;
    //====================
    void send(const Packet& packet);
    //====================
    void startReceivingMessages(ReceiveHandler receive_handler);
    void stopReceivingMessages();
    //====================
    std::size_t getId() const noexcept;
    const Endpoint& getEndpoint() const;
    //====================
  private:
    //====================
    using SharedPointer = std::shared_ptr<Connection>;
    //====================
    const std::size_t _id;
    static std::size_t getNextId();
    //====================
    boost::asio::io_context& _io_context;
    boost::asio::ip::tcp::socket _socket;
    std::unique_ptr<Endpoint> _connect_endpoint;

    std::atomic<bool> _is_closed{false};
    //====================
    static base::Bytes _read_buffer;
    std::atomic<bool> _is_receiving_enabled{false};
    std::mutex _receive_handler_mutex;
    ReceiveHandler _receive_handler;
    void receiveOne();
    //====================
    std::queue<base::Bytes> _pending_send_messages;
    std::mutex _pending_send_messages_mutex;
    void sendPendingMessages();
    //====================
};


} // namespace net