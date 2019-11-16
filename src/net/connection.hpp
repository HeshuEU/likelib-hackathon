#pragma once

#include "base/bytes.hpp"
#include "net/endpoint.hpp"
#include "net/packet.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <atomic>
#include <functional>
#include <memory>
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

    using ReceiveHandler = std::function<void(std::shared_ptr<Connection>, const Packet&)>;
    //====================
    Connection(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket&& socket, ReceiveHandler&& receive_handler);

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
    void startSession();
    void close();
    //====================
    void send(const Packet& packet);
    //====================
    std::size_t getId() const noexcept;
    const Endpoint& getEndpoint() const;
  private:
    //====================
    using SharedPointer = std::shared_ptr<Connection>;
    //====================
    const std::size_t _id;
    static std::size_t getNextId();
    //====================
    boost::asio::io_context& _io_context;
    boost::asio::ip::tcp::socket _socket;
    std::unique_ptr<Endpoint> _network_address;

    bool _is_closed{false};
    //====================
    static base::Bytes _read_buffer;
    std::atomic<bool> _is_receiving_enabled{false};
    ReceiveHandler _receive_handler;
    void receiveOne();

    void startReceivingMessages();
    void stopReceivingMessages();
    //====================
    std::queue<base::Bytes> _pending_send_messages;
    void send(base::Bytes&& data);
    void sendPendingMessages();
    //====================
};


} // namespace net