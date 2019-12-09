#pragma once

#include "net/endpoint.hpp"
#include "net/peer.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <functional>

namespace net
{

class Connector
{
  public:
    Connector(boost::asio::io_context& io_context, Peers& peers);

    void connect(const Endpoint& address, std::function<void(Peer&)> on_connect);

  private:
    boost::asio::io_context& _io_context;

    Peers& _peers;
};

} // namespace net