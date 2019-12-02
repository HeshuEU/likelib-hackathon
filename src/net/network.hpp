#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "net/connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

#include <list>
#include <memory>
#include <set>
#include <thread>

namespace net
{

class NetworkHandler
{
  public:
    virtual void onBlockReceived(bc::Block&&) = 0;
    virtual void onTransactionReceived(bc::Transaction&&) = 0;
};

class Network
{
  public:
    //===================
    Network(const base::PropertyTree& config, NetworkHandler& handler);
    ~Network();
    //===================
    void run();
    void waitForFinish();
    //===================
    void connect(const Endpoint& address);
    void connect(const std::vector<Endpoint>& nodes);
    //===================
    void broadcastBlock(const bc::Block& block);
    void broadcastTransaction(const bc::Transaction& tx);
    //===================
  private:
    //===================
    const Endpoint _listen_ip;
    const unsigned short _server_public_port;
    //===================
    boost::asio::io_context _io_context;
    std::list<std::shared_ptr<Connection>> _connections;
    //===================
    std::unique_ptr<std::thread> _network_thread;
    void networkThreadWorkerFunction() noexcept;
    //===================
    std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    void acceptClients();
    void acceptLoop();
    //===================
    boost::asio::steady_timer _heartbeat_timer;
    std::set<std::size_t> _not_ponged_peer_ids;
    void scheduleHeartBeat();
    void dropZombieConnections();
    //===================
    void connectionReceivedPacketHandler(Connection& connection, const Packet& packet);
    //===================
    NetworkHandler& _handler;
};

} // namespace net