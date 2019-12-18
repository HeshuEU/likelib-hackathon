#pragma once

#include "net/connection.hpp"
#include "net/endpoint.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <functional>

namespace net
{

class Connector
{
  public:
    //===================
    explicit Connector(boost::asio::io_context& io_context);
    //===================
    void connect(const Endpoint& address, std::function<void(std::unique_ptr<Connection>)> on_connect);
    //===================
  private:
    //===================
    boost::asio::io_context& _io_context;
    //===================
};

} // namespace net