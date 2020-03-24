#pragma once

#include "base/property_tree.hpp"
#include "block.hpp"
#include "lk/peer.hpp"
#include "net/acceptor.hpp"
#include "net/connector.hpp"
#include "net/session.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

#include <list>
#include <memory>
#include <set>
#include <shared_mutex>
#include <thread>

namespace lk
{


class PeerTable
{
  public:
    //=================================
    PeerTable(lk::Address host_id);
    //=================================
    void tryAddPeer(std::unique_ptr<Peer>);
    //=================================
  private:
    //=================================
    const lk::Address _host_id;
    //=================================
    static constexpr std::size_t MAX_BUCKET_SIZE = 10;
    std::array<std::vector<std::unique_ptr<Peer>>, lk::Address::LENGTH_IN_BYTES * 8> _buckets;
    //=================================
};


// Kademlia based host
class Host
{
  public:
    //=================================
    explicit Host(const base::PropertyTree& config, std::size_t connections_limit);
    ~Host();
    //=================================
    void checkOutPeer(const net::Endpoint& address);
    bool isConnectedTo(const net::Endpoint& endpoint) const;
    //=================================
    void broadcast(const base::Bytes& data);
    //=================================
    void run();
    void join();
    //=================================
    void forEachPeer(std::function<void(const Peer&)> f);
    std::vector<Peer::Info> allConnectedPeersInfo() const;
    //=================================
  private:
    //=================================
    const base::PropertyTree& _config;
    //=================================
    const net::Endpoint _listen_ip;
    const unsigned short _server_public_port;
    const std::size_t _max_connections_number;
    //=================================
    boost::asio::io_context _io_context;
    //===================
    std::thread _network_thread;
    void networkThreadWorkerFunction() noexcept;
    //=================================
    std::vector<Peer> _connected_peers;
    mutable std::shared_mutex _connected_peers_mutex;
    net::Session& addNewSession(std::unique_ptr<net::Connection> peer);

    boost::asio::steady_timer _heartbeat_timer;
    void scheduleHeartBeat();
    void dropZombiePeers();
    //=================================
    net::Acceptor _acceptor;
    void accept();
    net::Connector _connector;
    //=================================
};

} // namespace net