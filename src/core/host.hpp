#pragma once

#include "core/block.hpp"
#include "core/peer.hpp"

#include "net/acceptor.hpp"
#include "net/connector.hpp"
#include "net/session.hpp"

#include "base/property_tree.hpp"

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

class Core;

class PeerTable : public PeerPoolBase
{
  public:
    //=================================
    explicit PeerTable(lk::Address host_address);
    //=================================
    /*
     * Trying to add peer to a table. If succeeded, then std::unique_ptr is moved; if failed --
     * the value is unchanged.
     * @returns true if succeeded, false otherwise
     * @threadsafe
     */
    bool tryAddPeer(std::shared_ptr<PeerBase> peer) override;

    void removePeer(std::shared_ptr<PeerBase> peer) override;

    void removePeer(PeerBase* peer) override;

    // thread-safe
    void removeSilent();
    //=================================

    // thread-safe
    void forEachPeer(std::function<void(const PeerBase&)> f) const override;
    void forEachPeer(std::function<void(PeerBase&)> f) override;

    void broadcast(const base::Bytes& data) override;
    //=================================

    std::vector<Peer::Info> allPeersInfo() const override;

  private:
    //=================================
    const lk::Address _host_address;
    //=================================
    static constexpr std::size_t MAX_BUCKET_SIZE = 10;
    std::array<std::vector<std::shared_ptr<PeerBase>>, lk::Address::ADDRESS_BYTES_LENGTH * 8> _buckets;
    mutable std::shared_mutex _buckets_mutex;
    //=================================
    std::size_t calcBucketIndex(const lk::Address& peer_address);

    /*
     * Returns the least recently seen peer in a bucket.
     * @throws base::AssertionFailed if there is no such bucket or the bucket is empty
     * Thread unsafe! The call must be protected by a lock.
     */
    std::size_t getLeastRecentlySeenPeerIndex(std::size_t bucket_id);


    /*
     * Removes given peer from bucket.
     * Thread unsafe!
     */
    void removePeer(std::size_t bucket_index, std::size_t peer_index);

    //=================================
};


// Kademlia-based host
class Host
{
  public:
    //=================================
    explicit Host(const base::PropertyTree& config, std::size_t connections_limit, lk::Core& core);
    ~Host();
    //=================================
    void checkOutPeer(const net::Endpoint& address);
    bool isConnectedTo(const net::Endpoint& endpoint) const;
    PeerTable& getPool() noexcept;
    const PeerTable& getPool() const noexcept;
    //=================================
    void broadcast(const base::Bytes& data);
    void broadcast(const lk::Block& block);
    void broadcast(const lk::Transaction& tx);
    //=================================
    void run();
    void join();
    //=================================
    std::vector<Peer::Info> allConnectedPeersInfo() const;
    unsigned short getPublicPort() const noexcept;
    //=================================
  private:
    //=================================
    const base::PropertyTree& _config;
    //=================================
    const net::Endpoint _listen_ip;
    const unsigned short _server_public_port;
    const std::size_t _max_connections_number;
    lk::Core& _core;
    //=================================
    boost::asio::io_context _io_context;
    //===================
    std::thread _network_thread;
    void networkThreadWorkerFunction() noexcept;
    //=================================
    PeerTable _connected_peers;

    boost::asio::steady_timer _heartbeat_timer;
    void scheduleHeartBeat();
    void dropZombiePeers();
    //=================================
    net::Acceptor _acceptor;
    void accept();
    void onAccept(std::unique_ptr<net::Connection> connection);

    net::Connector _connector;
    void onConnect(std::unique_ptr<net::Connection> connection);
    //=================================
};

} // namespace net