#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>

namespace base::network
{


class ConnectionAcceptor
{
  public:
    ConnectionAcceptor(boost::asio::io_context& io_context, const boost::asio::ip::address& bind_ip);

    void startAcceptLoop();

    void stopAcceptLoop();

  private:
    void _handleAccept();

    boost::asio::io_context& _io_context;
    boost::asio::ip::address _bind_ip;
};


} // namespace base::network