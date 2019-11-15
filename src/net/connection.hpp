#pragma once

#include "base/bytes.hpp"
#include "endpoint.hpp"

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
    void close();
    //====================
    void ping();
    //====================
    using PongHandler = std::function<void()>;
    void onPong(const PongHandler& handler);
    void onPong(PongHandler&& handler);

    using DataHandler = std::function<void(const base::Bytes& buffer, std::size_t bytes_received)>;
    void onData(const DataHandler& handler);
    void onData(DataHandler&& handler);
    //====================

    const Endpoint& getEndpoint() const;
    //====================
  private:
    //====================
    using SharedPointer = std::shared_ptr<Connection>;
    //====================
    boost::asio::io_context& _io_context;
    boost::asio::ip::tcp::socket _socket;
    std::unique_ptr<Endpoint> _network_address;

    bool _is_closed{false};
    //====================
    using ReadHandler = std::function<void(const base::Bytes& message, std::size_t bytes_received)>;
    static base::Bytes _read_buffer;
    std::atomic<bool> _is_receiving_enabled{false};
    std::unique_ptr<ReadHandler> _on_receive;

    void receiveOne();
    void setOnReceive(ReadHandler handler);

    void startReceivingMessages();
    void stopReceivingMessages();
    //====================
    std::queue<base::Bytes> _pending_send_messages;
    void sendPendingMessages();
    void send(base::Bytes&& data);
    //====================
    std::size_t _non_responded_pings{0};
    //====================
    std::unique_ptr<PongHandler> _on_pong;
    std::unique_ptr<DataHandler> _on_data;
    //====================
};


} // namespace net