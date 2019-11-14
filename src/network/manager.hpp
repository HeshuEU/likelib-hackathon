#pragma once

#include "connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <deque>
#include <memory>
#include <thread>

namespace network
{

class Manager
{
  public:
    //===================
    Manager(const NetworkAddress& listen_ip);

    ~Manager();
    //===================
    void run();

    void connect(const NetworkAddress& address);
    void connect(const std::vector<NetworkAddress>& nodes);

    void waitForFinish();

  private:
    void networkThreadWorkerFunction() noexcept;
    std::unique_ptr<std::thread> _network_thread;

    void acceptClients();

    boost::asio::io_context _io_context;

    std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    const NetworkAddress& _listen_ip;

    void acceptLoop();

    std::queue<std::unique_ptr<Connection>> _connections;
};


} // namespace network