#pragma once

#include "session.hpp"

#include "base/property_tree.hpp"

#include <boost/beast/core.hpp>
#include <boost/thread.hpp>

namespace websocket
{

class WebSocketAcceptor
{
  public:
    using ConnectionRegistration = std::function<void(boost::asio::ip::tcp::socket&& socket)>;

    explicit WebSocketAcceptor(const base::PropertyTree& config, ConnectionRegistration registration);
    ~WebSocketAcceptor();

    void run();
    void stop();

  private:
    const base::PropertyTree& _config;
    ConnectionRegistration _connection_registration;

    boost::asio::io_context _io_context;
    boost::asio::ip::tcp::endpoint _endpoint;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::thread _network_thread;

    void accept();
    void networkThreadWorkerFunction() noexcept;
};

}