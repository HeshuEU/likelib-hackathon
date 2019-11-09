#pragma once

#include "base/bytes.hpp"
#include "base/network/network_address.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <functional>
#include <memory>
#include <queue>

namespace base::network
{

class Connection
{
  public:
    //====================
    Connection(boost::asio::ip::tcp::socket&& socket);

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection& other) = delete;

    Connection(Connection&&) = default;
    Connection& operator=(Connection&&) = delete;

    ~Connection();
    //====================
    using ReadHandler = std::function<void(const base::Bytes&)>;

    void send(base::Bytes&& data);

    void setOnReceive(ReadHandler handler);

    const NetworkAddress& getRemoteNetworkAddress() const;

    boost::asio::ip::tcp::socket& getSocket(); // TODO: try come up with something better

  private:
    boost::asio::io_context& _io_context;
    boost::asio::ip::tcp::socket _socket;
    std::unique_ptr<NetworkAddress> _network_address;

    std::unique_ptr<ReadHandler> _on_receive;
    void _sendHandler(const boost::system::error_code& error, std::size_t bytes_sent);

    void _receiveOne();
    void _receiveHandler(const boost::system::error_code& error, std::size_t bytes_received);

    std::queue<base::Bytes> _write_pending_messages;

    void _sendPendingMessages();
};


} // namespace base::network