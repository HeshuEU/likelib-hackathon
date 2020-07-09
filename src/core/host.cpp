#include "host.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "core/core.hpp"

#include <boost/asio/error.hpp>

#include <algorithm>
#include <chrono>
#include <iterator>

namespace ba = boost::asio;

namespace lk
{


bool BasicPeerPool::tryAddPeer(std::shared_ptr<Peer> peer)
{
    std::unique_lock lk(_pool_mutex);
    if (_pool.find(peer.get()) != _pool.end()) {
        return false;
    }
    _pool.insert({ peer.get(), std::move(peer) });
    return true;
}


bool BasicPeerPool::tryRemovePeer(const Peer* peer)
{
    std::unique_lock lk(_pool_mutex);
    if (auto it = _pool.find(peer); it != _pool.end()) {
        _pool.erase(it);
        return true;
    }
    else {
        return false;
    }
}


void BasicPeerPool::forEachPeer(std::function<void(const Peer&)> f) const
{
    std::shared_lock lk(_pool_mutex);
    for (const auto& p : _pool) {
        f(*p.second);
    }
}


void BasicPeerPool::forEachPeer(std::function<void(Peer&)> f)
{
    std::unique_lock lk(_pool_mutex);
    for (const auto& p : _pool) {
        f(*p.second);
    }
}


bool BasicPeerPool::hasPeerWithEndpoint(const net::Endpoint& endpoint) const
{
    bool have_peer_with_endpoint = false;
    forEachPeer([&have_peer_with_endpoint, endpoint](const Peer& peer) {
        if (peer.getPublicEndpoint() == endpoint || peer.getEndpoint() == endpoint) {
            have_peer_with_endpoint = true;
        }
    });
    return have_peer_with_endpoint;
}


KademliaPeerPool::KademliaPeerPool(lk::Address host_address)
  : _host_address{ std::move(host_address) }
{}


std::size_t KademliaPeerPool::calcDifference(const base::FixedBytes<lk::Address::LENGTH_IN_BYTES>& a,
                                             const base::FixedBytes<lk::Address::LENGTH_IN_BYTES>& b)
{
    std::size_t byte_index = 0;
    while (byte_index < lk::Address::LENGTH_IN_BYTES && a[byte_index] == b[byte_index]) {
        ++byte_index;
    }

    if (byte_index == lk::Address::LENGTH_IN_BYTES) {
        return lk::Address::LENGTH_IN_BYTES * 8;
    }
    else {
        base::Byte byte_a = a[byte_index], byte_b = b[byte_index];

        for (std::size_t bit_index = 0; bit_index < 8; ++bit_index) {
            if ((byte_a & 0b10000000) != (byte_b & 0b10000000)) {
                return byte_index * 8 + bit_index;
            }
            byte_a <<= 1;
            byte_b <<= 1;
        }

        ASSERT(false); // must be unreachable
        return 0;
    }
}


std::size_t KademliaPeerPool::calcBucketIndex(const lk::Address& peer_address) const
{
    const auto& a = _host_address.getBytes();
    const auto& b = peer_address.getBytes();
    return calcDifference(a, b);
}


std::size_t KademliaPeerPool::getLeastRecentlySeenPeerIndex(const std::size_t bucket_id)
{
    ASSERT(_buckets.size() >= bucket_id);
    ASSERT(!_buckets[bucket_id].empty());

    ASSERT(_buckets[bucket_id][0]);
    std::size_t lrs_index = 0;
    for (std::size_t i = 1; i < _buckets[bucket_id].size(); ++i) {
        ASSERT(_buckets[bucket_id][i]);
        if (_buckets[bucket_id][lrs_index]->getLastSeen() > _buckets[bucket_id][i]->getLastSeen()) {
            lrs_index = i;
        }
    }

    return lrs_index;
}


bool KademliaPeerPool::tryAddPeer(std::shared_ptr<Peer> peer)
{
    std::unique_lock lk{ _buckets_mutex };

    for (const auto& bucket : _buckets) {
        for (const auto& p : bucket) {
            if (p->getPublicEndpoint() == peer->getPublicEndpoint() || p->getEndpoint() == peer->getEndpoint()) {
                return false;
            }
        }
    }

    std::size_t bucket_index = calcBucketIndex(peer->getAddress());
    if (bucket_index == Address::LENGTH_IN_BYTES * 8) {
        return false;
    }
    if (_buckets[bucket_index].size() < MAX_BUCKET_SIZE) {
        // accept peer
        _buckets[bucket_index].push_back(std::move(peer));
        return true;
    }
    else {
        std::size_t index = getLeastRecentlySeenPeerIndex(bucket_index);
        auto quiet_for = base::Time::now().getSeconds() - _buckets[bucket_index][index]->getLastSeen().getSeconds();
        if (quiet_for > base::config::NET_PING_FREQUENCY + base::config::NET_CONNECT_TIMEOUT) {
            removePeer(bucket_index, index);
            _buckets[bucket_index].push_back(std::move(peer));
            return true;
        }
        else {
            return false;
        }
    }
}


bool KademliaPeerPool::tryRemovePeer(const Peer* peer)
{
    std::unique_lock lk{ _buckets_mutex };
    for (auto& bucket : _buckets) {
        if (auto it =
              std::find_if(bucket.cbegin(), bucket.cend(), [peer](const auto& cand) { return peer == cand.get(); });
            it != bucket.cend()) {
            bucket.erase(it);
            return true;
        }
    }

    return false;
}


void KademliaPeerPool::removePeer(std::size_t bucket_index, std::size_t peer_index)
{
    ASSERT(bucket_index < MAX_BUCKET_SIZE);
    ASSERT(peer_index < _buckets[bucket_index].size());

    auto it = _buckets[bucket_index].begin();
    std::advance(it, peer_index);
    _buckets[bucket_index].erase(it);
}


void KademliaPeerPool::removeSilent()
{
    // TODO: rework this
    std::unique_lock lk(_buckets_mutex);
    for (auto& bucket : _buckets) {
        bucket.erase(
          std::remove_if(bucket.begin(), bucket.end(), [](const auto& peer) { return peer->isSessionClosed(); }),
          bucket.end());
    }
}


void KademliaPeerPool::forEachPeer(std::function<void(const Peer&)> f) const
{
    std::shared_lock lk(_buckets_mutex);
    for (const auto& bucket : _buckets) {
        for (const auto& peer : bucket) {
            f(*peer);
        }
    }
}


void KademliaPeerPool::forEachPeer(std::function<void(Peer&)> f)
{
    std::unique_lock lk(_buckets_mutex);
    for (auto& bucket : _buckets) {
        for (auto& peer : bucket) {
            f(*peer);
        }
    }
}


std::vector<msg::NodeIdentityInfo> KademliaPeerPool::lookup(const lk::Address& address, std::size_t alpha)
{
    auto ret = allPeersInfo();
    std::sort(ret.begin(), ret.end(), [address](const auto& a, const auto& b) {
        return calcDifference(a.address.getBytes(), address.getBytes()) <
               calcDifference(b.address.getBytes(), address.getBytes());
    });

    if (ret.size() > alpha) {
        ret.erase(ret.begin() + alpha, ret.end());
    }
    return ret;
}


bool KademliaPeerPool::hasPeerWithEndpoint(const net::Endpoint& endpoint) const
{
    bool have_peer_with_endpoint = false;
    forEachPeer([&have_peer_with_endpoint, endpoint](const Peer& peer) {
        if (peer.getPublicEndpoint() == endpoint || peer.getEndpoint() == endpoint) {
            have_peer_with_endpoint = true;
        }
    });
    return have_peer_with_endpoint;
}


Host::Host(const base::PropertyTree& config, std::size_t connections_limit, lk::Core& core)
  : _config{ config }
  , _listen_ip{ _config.get<std::string>("net.listen_addr") }
  , _server_public_port{ _config.get<unsigned short>("net.public_port") }
  , _max_connections_number{ connections_limit }
  , _core{ core }
  , _handshaked_peers{ core.getThisNodeAddress() }
  , _heartbeat_timer{ _io_context }
  , _acceptor{ _io_context, _listen_ip }
  , _connector{ _io_context }
{}


Host::~Host()
{
    _io_context.stop();
}


void Host::scheduleHeartBeat()
{
    _heartbeat_timer.expires_after(std::chrono::seconds(base::config::NET_PING_FREQUENCY));
    _heartbeat_timer.async_wait([this](const boost::system::error_code& ec) {
        dropZombiePeers();
        scheduleHeartBeat();
    });
}


void Host::accept()
{
    _acceptor.accept([this](std::unique_ptr<net::Connection> connection) {
        ASSERT(connection);
        onAccept(std::move(connection));
        accept();
    });
}


void Host::onAccept(std::unique_ptr<net::Connection> connection)
{
    auto session = std::make_shared<net::Session>(std::move(connection));
    lk::Peer::accepted(std::move(session), _rating_manager.get(session->getEndpoint()), Peer::Context{ _core, *this });
}


void Host::checkOutPeer(const net::Endpoint& endpoint, const lk::Address& address)
{
    if (_core.getThisNodeAddress() == address) {
        return;
    }

    if (_listen_ip == endpoint) {
        return;
    }

    if (isConnectedTo(endpoint)) {
        return;
    }

    {
        std::lock_guard lk(_conntextor_set_mutex);
        auto it = _connector_in_process.find(endpoint);
        if (it != _connector_in_process.end()) {
            return;
        }
        else {
            _connector_in_process.insert(endpoint);
        }
    }

    if (!_rating_manager.get(endpoint)) {
        return;
    }
    LOG_DEBUG << "Connecting to node " << endpoint
              << " that has good rating = " << _rating_manager.get(endpoint).getValue();
    _connector.connect(
      endpoint,
      base::config::NET_CONNECT_TIMEOUT,
      [this, endpoint](std::unique_ptr<net::Connection> connection) {
          ASSERT(connection);
          std::lock_guard lk(_conntextor_set_mutex);
          auto it = _connector_in_process.find(endpoint);
          if (it != _connector_in_process.end()) {
              _connector_in_process.erase(it);
          }
          auto session = std::make_shared<net::Session>(std::move(connection));
          lk::Peer::connected(
            std::move(session), _rating_manager.get(session->getEndpoint()), Peer::Context{ _core, *this });
      },
      [this, endpoint](const net::Connector::ConnectError&) {
          // TODO: error handling
          std::lock_guard lk(_conntextor_set_mutex);
          auto it = _connector_in_process.find(endpoint);
          if (it != _connector_in_process.end()) {
              _connector_in_process.erase(it);
          }
      });
    LOG_DEBUG << "Connection to " << endpoint << " is added to queue";
}


RatingManager& Host::getRatingManager() noexcept
{
    return _rating_manager;
}


BasicPeerPool& Host::getNonHandshakedPool() noexcept
{
    return _non_handshaked_peers;
}


KademliaPeerPool& Host::getHandshakedPool() noexcept
{
    return _handshaked_peers;
}


boost::asio::io_context& Host::getIoContext() noexcept
{
    return _io_context;
}


void Host::networkThreadWorkerFunction() noexcept
{
    try {
        _io_context.run();
    }
    catch (const std::exception& e) {
        // TODO: thread worker function error-handling
        LOG_WARNING << "Error occurred in network thread: " << e.what();
    }
    catch (...) {
        // global catch done for safety, since thread function cannot throw.
        LOG_WARNING << "Error occurred in network thread";
    }
}


void Host::run()
{
    LOG_INFO << "Listening for incoming connections on " << _listen_ip;
    accept();

    scheduleHeartBeat();
    _network_thread = std::thread(&Host::networkThreadWorkerFunction, this);

    bootstrap();
}


void Host::bootstrap()
{
    if (_config.hasKey("nodes")) {
        for (const auto& node : _config.getVector<std::string>("nodes")) {
            checkOutPeer(net::Endpoint(node));
        }
    }
}


void Host::join()
{
    if (_network_thread.joinable()) {
        _network_thread.join();
    }
}


void Host::dropZombiePeers()
{
    _handshaked_peers.removeSilent();
}


void Host::broadcast(const ImmutableBlock& block)
{
    _handshaked_peers.forEachPeer([&block](Peer& peer) { peer.sendBlock(block); });
}


void Host::broadcastNewBlock(const ImmutableBlock& block)
{
    _handshaked_peers.forEachPeer([&block](Peer& peer) { peer.sendNewBlock(block); });
}


void Host::broadcast(const Transaction& tx)
{
    _handshaked_peers.forEachPeer([tx](Peer& peer) { peer.sendTransaction(tx); });
}


bool Host::isConnectedTo(const net::Endpoint& endpoint) const
{
    return _non_handshaked_peers.hasPeerWithEndpoint(endpoint) || _handshaked_peers.hasPeerWithEndpoint(endpoint);
}


std::vector<msg::NodeIdentityInfo> Host::allConnectedPeersInfo() const
{
    return _handshaked_peers.allPeersInfo();
}


unsigned short Host::getPublicPort() const noexcept
{
    return _server_public_port;
}

} // namespace net
