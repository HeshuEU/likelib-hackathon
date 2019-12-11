#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "net/acceptor.hpp"
#include "net/connector.hpp"
#include "net/peer.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

#include <list>
#include <memory>
#include <set>
#include <shared_mutex>
#include <thread>

namespace net
{

class Host
{
  public:
    //===================
    Host(const base::PropertyTree& config);
    ~Host();
    //===================
    void accept(std::function<void(std::unique_ptr<Peer>)> on_accept);
    //===================
    void connect(const Endpoint& address, std::function<void(std::unique_ptr<Peer>)> on_connect);
    void connect(const std::vector<Endpoint>& nodes, std::function<void(std::unique_ptr<Peer>)> on_connect);
    //===================
    void broadcast(const base::Bytes& data);
    //===================
    void run();
    void join();
    //===================
  private:
    //===================
    const Endpoint _listen_ip;
    const unsigned short _server_public_port;
    //===================
    boost::asio::io_context _io_context;
    //===================
    std::thread _network_thread;
    void networkThreadWorkerFunction() noexcept;
    //===================
    Peers _peers;
    net::Acceptor _acceptor;
    net::Connector _connector;

    void acceptClients();
    //===================
    boost::asio::steady_timer _heartbeat_timer;
    void scheduleHeartBeat();
    void dropZombieConnections();
    //===================
};

} // namespace net