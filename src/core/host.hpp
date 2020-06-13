#pragma once

#include "base/database.hpp"
#include "base/property_tree.hpp"
#include "core/block.hpp"
#include "core/peer.hpp"
#include "core/rating.hpp"
#include "net/acceptor.hpp"
#include "net/connector.hpp"
#include "net/session.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <shared_mutex>
#include <thread>

namespace lk
{

class Core;

class BasicPeerPool : public PeerPoolBase
{
  public:
    bool tryAddPeer(std::shared_ptr<Peer> peer) override;

    bool tryRemovePeer(const Peer* peer) override;
    //=================================

    // thread-safe
    void forEachPeer(std::function<void(const Peer&)> f) const override;
    void forEachPeer(std::function<void(Peer&)> f) override;

    bool hasPeerWithEndpoint(const net::Endpoint& endpoint) const override;
    //=================================
  private:
    std::map<const Peer*, std::shared_ptr<Peer>> _pool;
    mutable std::shared_mutex _pool_mutex;
};


class KademliaPeerPool : public KademliaPeerPoolBase
{
  public:
    //=================================
    explicit KademliaPeerPool(lk::Address host_address);
    //=================================
    /*
     * Trying to add peer to a table. If succeeded, then std::unique_ptr is moved; if failed --
     * the value is unchanged.
     * @returns true if succeeded, false otherwise
     * @threadsafe
     */
    bool tryAddPeer(std::shared_ptr<Peer> peer) override;

    bool tryRemovePeer(const Peer* peer) override;

    // thread-safe
    void removeSilent();
    //=================================

    // thread-safe
    void forEachPeer(std::function<void(const Peer&)> f) const override;
    void forEachPeer(std::function<void(Peer&)> f) override;

    bool hasPeerWithEndpoint(const net::Endpoint& endpoint) const override;
    //=================================

    std::vector<msg::NodeIdentityInfo> lookup(const lk::Address& address, std::size_t alpha) override;

  private:
    //=================================
    const lk::Address _host_address;
    //=================================
    static constexpr std::size_t MAX_BUCKET_SIZE = 10;
    std::array<std::vector<std::shared_ptr<Peer>>, lk::Address::LENGTH_IN_BYTES * 8> _buckets;
    mutable std::shared_mutex _buckets_mutex;
    //=================================
    static std::size_t calcDifference(const base::FixedBytes<lk::Address::LENGTH_IN_BYTES>& a,
                                      const base::FixedBytes<lk::Address::LENGTH_IN_BYTES>& b);

    std::size_t calcBucketIndex(const lk::Address& peer_address) const;

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
    void checkOutPeer(const net::Endpoint& endpoint, const lk::Address& address = lk::Address::null());
    bool isConnectedTo(const net::Endpoint& endpoint) const;

    RatingManager& getRatingManager() noexcept;
    BasicPeerPool& getNonHandshakedPool() noexcept;
    KademliaPeerPool& getHandshakedPool() noexcept;
    //=================================
    void broadcast(const ImmutableBlock& block);
    void broadcastNewBlock(const ImmutableBlock& block);
    void broadcast(const lk::Transaction& tx);
    //=================================
    void run();
    void join();
    //=================================
    std::vector<msg::NodeIdentityInfo> allConnectedPeersInfo() const;
    unsigned short getPublicPort() const noexcept;
    boost::asio::io_context& getIoContext() noexcept;
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
    RatingManager _rating_manager{ _config };

    BasicPeerPool _non_handshaked_peers;
    KademliaPeerPool _handshaked_peers;
    void bootstrap();

    boost::asio::steady_timer _heartbeat_timer;
    void scheduleHeartBeat();
    void dropZombiePeers();
    //=================================
    net::Acceptor _acceptor;
    void accept();
    void onAccept(std::unique_ptr<net::Connection> connection);

    net::Connector _connector;
    std::set<net::Endpoint> _connector_in_process;
    std::mutex _conntextor_set_mutex;
    //=================================
};

} // namespace net
