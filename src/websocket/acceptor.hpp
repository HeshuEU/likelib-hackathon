#pragma once

#include "session.hpp"

#include <boost/beast/core.hpp>
#include <boost/thread.hpp>

namespace websocket
{

class WebSocketAcceptor
{
    using SocketRegistration = std::function<void(boost::asio::ip::tcp::socket&& socket)>;

  public:
    explicit WebSocketAcceptor(rapidjson::Value config, SocketRegistration registration);
    ~WebSocketAcceptor() noexcept;

    void run();
    void stop() noexcept;

  private:
    rapidjson::Value _config;
    SocketRegistration _connectionRegistration;

    boost::asio::io_context _io_context;
    boost::asio::ip::tcp::endpoint _endpoint;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::thread _inputNetworkThread;

    void accept();
    void networkThreadWorkerFunction() noexcept;
};

}