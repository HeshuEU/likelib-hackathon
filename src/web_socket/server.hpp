#pragma once

#include "session.hpp"

#include "base/property_tree.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>

#include <memory>
#include <thread>


namespace web_socket
{

class WebSocketServer
{
  public:
    explicit WebSocketServer(const base::PropertyTree& config);
    ~WebSocketServer();

    void run(RequestCall on_request);
    void stop();

  private:
    const base::PropertyTree& _config;
    boost::asio::io_context _io_context;
    boost::asio::ip::tcp::endpoint _endpoint;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::thread _network_thread;

    void do_accept(RequestCall on_request);
    void accept(RequestCall on_request);
    void networkThreadWorkerFunction() noexcept;
};

}