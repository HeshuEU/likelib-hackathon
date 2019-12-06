#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
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
    using DataHandler = std::function<void(base::Bytes&&)>;
    //===================
    Host(const base::PropertyTree& config, DataHandler handler);
    ~Host();
    //===================
    void run();
    void waitForFinish();
    //===================
    void connect(const Endpoint& address);
    void connect(const std::vector<Endpoint>& nodes);
    //===================
    void broadcastDataPacket(const base::Bytes& data);
    //===================
  private:
    //===================
    const Endpoint _listen_ip;
    const unsigned short _server_public_port;
    //===================
    boost::asio::io_context _io_context;
    std::list<Peer> _peers;
    std::shared_mutex _connections_mutex;
    //===================
    std::unique_ptr<std::thread> _network_thread;
    void networkThreadWorkerFunction() noexcept;
    //===================
    std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    void acceptClients();
    void acceptLoop();
    //===================
    boost::asio::steady_timer _heartbeat_timer;
    void scheduleHeartBeat();
    void dropZombieConnections();
    //===================
    void onConnectionReceivedPacketHandler(Packet&& packet);
    DataHandler _data_handler;
    //===================
};

} // namespace net