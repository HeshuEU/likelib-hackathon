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
    Acceptor(boost::asio::io_context& io_context, const Endpoint& listen_endpoint, Peers& accepted_peers);
    //==============
    void acceptInfinite(std::function<void(Peer&)> on_accept);
    void stopAccept();
    //==============
  private:
    //==============
    boost::asio::io_context& _io_context;
    Endpoint _listen_endpoint;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::atomic<bool> _is_stopped{true};
    //==============
    Peers& _accepted_peers;
    std::function<void(Peer&)> _on_accept;
    //==============
    void acceptOneImpl();
    //==============
};


} // namespace net