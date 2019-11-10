#pragma once

#include "base/bytes.hpp"
#include "network_address.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <queue>

namespace network
{

class Connection
{
  public:
    //====================
    Connection(boost::asio::ip::tcp::socket&& socket);

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
    using ReadHandler = std::function<void(const base::Bytes&)>;

    void send(base::Bytes&& data);

    void setOnReceive(ReadHandler handler);

    void startReceivingMessages();
    void stopReceivingMessages();

    void startSession();

    const NetworkAddress& getRemoteNetworkAddress() const;

    boost::asio::ip::tcp::socket& getSocket(); // TODO: try come up with something better

  private:
    boost::asio::io_context& _io_context;
    boost::asio::ip::tcp::socket _socket;
    std::unique_ptr<NetworkAddress> _network_address;

    std::unique_ptr<ReadHandler> _on_receive;
    void _sendHandler(const boost::system::error_code& error, std::size_t bytes_sent);

    std::atomic<bool> _is_receiving_enabled{false};
    void _receiveOne();
    void _receiveHandler(const boost::system::error_code& error, std::size_t bytes_received);

    std::queue<base::Bytes> _pending_send_messages;

    void _sendPendingMessages();

    static base::Bytes _read_buffer;
};


} // namespace network