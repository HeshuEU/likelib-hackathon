#pragma once

#include "base/bytes.hpp"
#include "network_address.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <queue>

namespace net
{

class Connection
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

    void startSession();

    void ping(std::function<void()> on_pong);

    const NetworkAddress& getRemoteNetworkAddress() const;
    //====================
  private:
    //====================
    boost::asio::io_context& _io_context;
    boost::asio::ip::tcp::socket _socket;
    std::unique_ptr<NetworkAddress> _network_address;
    //====================
    using ReadHandler = std::function<void(const base::Bytes& message, std::size_t bytes_received)>;
    static base::Bytes _read_buffer;
    std::atomic<bool> _is_receiving_enabled{false};

    std::unique_ptr<ReadHandler> _on_receive;

    void receiveOne();
    void receiveHandler(const boost::system::error_code& error, std::size_t bytes_received);
    void setOnReceive(ReadHandler handler);

    void startReceivingMessages();
    void stopReceivingMessages();
    //====================
    std::queue<base::Bytes> _pending_send_messages;
    void sendPendingMessages();
    void sendHandler(const boost::system::error_code& error, std::size_t bytes_sent);
    void send(base::Bytes&& data);
    //====================
};


} // namespace net