#include "host.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "core/core.hpp"

#include <boost/asio/error.hpp>

#include <chrono>
#include <iterator>

namespace ba = boost::asio;

namespace lk
{

PeerTable::PeerTable(lk::Address host_address)
  : _host_address{ std::move(host_address) }
{}


std::size_t PeerTable::calcBucketIndex(const lk::Address& peer_address)
{
    const auto& a = _host_address.getBytes();
    const auto& b = peer_address.getBytes();

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
    }
}


std::size_t PeerTable::getLeastRecentlySeenPeerIndex(const std::size_t bucket_id)
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


bool PeerTable::tryAdd(std::unique_ptr<Peer>& peer)
{
    std::unique_lock lk{_buckets_mutex};
    std::size_t bucket_index = calcBucketIndex(peer->getAddress());
    if (_buckets[bucket_index].size() < MAX_BUCKET_SIZE) {
        // accept peer
        _buckets[bucket_index].push_back(std::move(peer));
        return true;
    }
    else {
        std::size_t index = getLeastRecentlySeenPeerIndex(bucket_index);
        auto quiet_for = base::Time::now().getSecondsSinceEpoch() -
                         _buckets[bucket_index][index]->getLastSeen().getSecondsSinceEpoch();
        if (quiet_for > base::config::NET_PING_FREQUENCY * 2) {
            removePeer(bucket_index, index);
            _buckets[bucket_index].push_back(std::move(peer));
            return true;
        }
        else {
            return false;
        }
    }
}


bool PeerTable::tryAdd(std::unique_ptr<Peer>&& peer)
{
    return tryAdd(peer);
}


void PeerTable::removePeer(std::size_t bucket_index, std::size_t peer_index)
{
    ASSERT(bucket_index < MAX_BUCKET_SIZE);
    ASSERT(peer_index < _buckets[bucket_index].size());

    auto it = _buckets[bucket_index].begin();
    std::advance(it, peer_index);
    _buckets[bucket_index].erase(it);
}


void PeerTable::removeSilent()
{
    std::unique_lock lk(_buckets_mutex);
    for(auto& bucket : _buckets) {
        bucket.erase(std::remove_if(bucket.begin(), bucket.end(), [](const auto& peer) { return peer->isClosed(); }), bucket.end());
    }
}


void PeerTable::forEachPeer(std::function<void(const Peer&)> f) const
{
    std::shared_lock lk(_buckets_mutex);
    for(const auto& bucket : _buckets) {
        for(const auto& peer : bucket) {
            f(*peer);
        }
    }
}


void PeerTable::forEachPeer(std::function<void(Peer&)> f)
{
    std::unique_lock lk(_buckets_mutex);
    for(auto& bucket : _buckets) {
        for(auto& peer : bucket) {
            f(*peer);
        }
    }
}


Host::Host(const base::PropertyTree& config, std::size_t connections_limit, lk::Core& core)
  : _config{ config }
  , _listen_ip{ _config.get<std::string>("net.listen_addr") }
  , _server_public_port{ _config.get<unsigned short>("net.public_port") }
  , _max_connections_number{ connections_limit }
  , _core{ core }
  , _connected_peers{ core.getThisNodeAddress() }
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
    std::unique_lock lk{ _connected_peers_mutex };
    auto session = std::make_unique<net::Session>(std::move(connection));
    _connected_peers.tryAdd(lk::Peer::accepted(std::move(session), _core, *this));
}


void Host::checkOutPeer(const net::Endpoint& endpoint)
{
    if (_listen_ip == endpoint) {
        return;
    }

    if (isConnectedTo(endpoint)) {
        return;
    }

    bool have_peer_with_endpoint = false;
    _connected_peers.forEachPeer([&have_peer_with_endpoint, endpoint](const Peer& peer) {
        if (auto ep = peer.getPublicEndpoint(); ep && *ep == endpoint) {
            have_peer_with_endpoint = true;
        }
    });
    if(have_peer_with_endpoint) {
        return;
    }

    LOG_DEBUG << "Connecting to node " << endpoint;
    _connector.connect(endpoint, [this](std::unique_ptr<net::Connection> connection) {
        ASSERT(connection);
        onConnect(std::move(connection));
    });
    LOG_DEBUG << "Connection to " << endpoint << " is added to queue";
}


void Host::onConnect(std::unique_ptr<net::Connection> connection)
{
    std::unique_lock lk{ _connected_peers_mutex };
    auto session = std::make_unique<net::Session>(std::move(connection));
    _connected_peers.tryAdd(lk::Peer::connected(std::move(session), _core, *this));
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
    accept();
    scheduleHeartBeat();
    _network_thread = std::thread(&Host::networkThreadWorkerFunction, this);

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
    std::unique_lock lk(_connected_peers_mutex);
    _connected_peers.removeSilent();
}


void Host::broadcast(const base::Bytes& data)
{
    LOG_DEBUG << "Broadcasting data size = " << data.size();
    _connected_peers.forEachPeer([data](Peer& peer) {
        peer.send(data);
    });
}


void Host::broadcast(const lk::Block& block)
{
    std::unique_lock lk(_connected_peers_mutex);
}


void Host::broadcast(const lk::Transaction& tx)
{
    std::unique_lock lk(_connected_peers_mutex);
}


bool Host::isConnectedTo(const net::Endpoint& endpoint) const
{
    bool have_peer_with_endpoint = false;
    _connected_peers.forEachPeer([&have_peer_with_endpoint, endpoint](const Peer& peer) {
        if (auto ep = peer.getPublicEndpoint(); ep && *ep == endpoint) {
            have_peer_with_endpoint = true;
        }
    });
    return have_peer_with_endpoint;
}


std::vector<Peer::Info> Host::allConnectedPeersInfo() const
{
    std::vector<Peer::Info> ret;
    _connected_peers.forEachPeer([&ret](const Peer& peer) {
        ret.push_back(peer.getInfo());
    });
    return ret;
}


unsigned short Host::getPublicPort() const noexcept
{
    return _server_public_port;
}

} // namespace net
