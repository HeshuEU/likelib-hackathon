#pragma once

#include "net/endpoint.hpp"
#include "net/peer.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace net
{

class Acceptor
{
  public:
    //==============
    Acceptor(boost::asio::io_context& io_context, const Endpoint& listen_endpoint);
    //==============
    void accept(std::function<void(std::unique_ptr<Peer>)> on_accept);
    //==============
  private:
    //==============
    boost::asio::io_context& _io_context;
    Endpoint _listen_endpoint;
    boost::asio::ip::tcp::acceptor _acceptor;
    //==============
};


} // namespace net