#pragma once

#include "base/network/connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>
#include <thread>

namespace base::network
{

class Manager
{
  public:
    void run();

    void acceptClients(const boost::asio::ip::tcp::endpoint& listen_ip);

    void waitForFinish();

  private:
    void _networkThreadWorkerFunction() noexcept;
    std::unique_ptr<std::thread> _network_thread;

    boost::asio::io_context _io_context;

    std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    void _acceptHandler(std::shared_ptr<Connection> connection, const boost::system::error_code&);
};


} // namespace base::network