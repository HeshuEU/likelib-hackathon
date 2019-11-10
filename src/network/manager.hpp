#pragma once

#include "connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>
#include <thread>
#include <deque>

namespace network
{

class Manager
{
  public:
    //===================
    Manager() = default;

    ~Manager();
    //===================
    void run();

    void acceptClients(const boost::asio::ip::tcp::endpoint& listen_ip);

    void waitForFinish();

  private:
    void _networkThreadWorkerFunction() noexcept;
    std::unique_ptr<std::thread> _network_thread;

    boost::asio::io_context _io_context;

    std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    void _acceptOne();
    void _acceptHandler(const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket);

    std::queue<std::unique_ptr<Connection>> _connections;
};


} // namespace network