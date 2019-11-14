#pragma once

#include "connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

#include <list>
#include <memory>
#include <set>
#include <thread>

namespace net
{

class Manager
{
  public:
    //===================
    Manager(const Endpoint& listen_ip);
    ~Manager();
    //===================
    void run();
    void waitForFinish();
    //===================
    void connect(const Endpoint& address);
    void connect(const std::vector<Endpoint>& nodes);
    //===================
  private:
    //===================
    boost::asio::io_context _io_context;
    std::list<std::unique_ptr<Connection>> _connections;
    //===================
    std::unique_ptr<std::thread> _network_thread;
    void networkThreadWorkerFunction() noexcept;
    //===================
    std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    const Endpoint& _listen_ip;
    void acceptClients();
    void acceptLoop();
    //===================
    boost::asio::steady_timer _heartbeatTimer;
    //std::set<Endpoint> _not_responded_peers;
    void scheduleHeartBeat();
    void dropZombieConnections();
    //===================
};

} // namespace net