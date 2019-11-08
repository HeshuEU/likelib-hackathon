#pragma once

#include "base/bytes.hpp"
#include "base/network/network_address.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>

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

    void send(const base::Bytes&);

    const NetworkAddress& getRemoteNetworkAddress() const;

    boost::asio::ip::tcp::socket& getSocket(); // TODO: try come up with something better

  private:
    boost::asio::ip::tcp::socket _socket;
    std::unique_ptr<NetworkAddress> _network_address;
};


} // namespace base::network