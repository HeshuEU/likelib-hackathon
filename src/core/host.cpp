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

PeerTable::PeerTable(lk::Address host_address)
  : _host_address{ std::move(host_address) }
{}


std::size_t PeerTable::calcDifference(const base::FixedBytes<lk::Address::LENGTH_IN_BYTES>& a,
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
    }
}


std::size_t PeerTable::calcBucketIndex(const lk::Address& peer_address) const
{
    const auto& a = _host_address.getBytes();
    const auto& b = peer_address.getBytes();
    return calcDifference(a, b);
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


bool PeerTable::tryAddPeer(std::shared_ptr<PeerBase> peer)
{
    std::unique_lock lk{ _buckets_mutex };
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


void PeerTable::removePeer(std::shared_ptr<PeerBase> peer)
{
    removePeer(peer.get());
}


void PeerTable::removePeer(PeerBase* peer)
{
    std::unique_lock lk{ _buckets_mutex };
    for (auto& bucket : _buckets) {
        bucket.erase(std::remove_if(
                       bucket.begin(), bucket.end(), [peer](const auto& candidate) { return peer == candidate.get(); }),
                     bucket.end());
    }
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
    for (auto& bucket : _buckets) {
        bucket.erase(std::remove_if(bucket.begin(), bucket.end(), [](const auto& peer) { return peer->isClosed(); }),
                     bucket.end());
    }
}


void PeerTable::forEachPeer(std::function<void(const PeerBase&)> f) const
{
    std::shared_lock lk(_buckets_mutex);
    for (const auto& bucket : _buckets) {
        for (const auto& peer : bucket) {
            f(*peer);
        }
    }
}


void PeerTable::forEachPeer(std::function<void(PeerBase&)> f)
{
    std::unique_lock lk(_buckets_mutex);
    for (auto& bucket : _buckets) {
        for (auto& peer : bucket) {
            f(*peer);
        }
    }
}


void PeerTable::broadcast(const base::Bytes& data)
{
    forEachPeer([data](PeerBase& peer) { peer.send(data); });
}


std::vector<PeerBase::Info> PeerTable::lookup(const lk::Address& address, const std::size_t alpha)
{
    auto ret = allPeersInfo();
    std::sort(ret.begin(), ret.end(), [this](const auto& a, const auto& b) {
        return calcDifference(a.address.getBytes(), _host_address.getBytes()) <
               calcDifference(b.address.getBytes(), _host_address.getBytes());
    });

    if (ret.size() > alpha) {
        ret.erase(ret.begin() + alpha, ret.end());
    }
    return ret;
}


std::vector<Peer::Info> PeerTable::allPeersInfo() const
{
    std::vector<Peer::Info> ret;
    forEachPeer([&ret](const PeerBase& peer) { ret.push_back(peer.getInfo()); });
    return ret;
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
    auto session = std::make_unique<net::Session>(std::move(connection));
    _connected_peers.tryAddPeer(lk::Peer::accepted(std::move(session), *this, _core));
}


void Host::checkOutPeer(const net::Endpoint& endpoint, std::function<void(std::shared_ptr<Peer>)> on_connect)
{
    if (_listen_ip == endpoint) {
        return;
    }

    if (isConnectedTo(endpoint)) {
        return;
    }

    bool have_peer_with_endpoint = false;
    _connected_peers.forEachPeer([&have_peer_with_endpoint, endpoint](const PeerBase& peer) {
        if (peer.getPublicEndpoint() == endpoint) {
            have_peer_with_endpoint = true;
        }
    });
    if (have_peer_with_endpoint) {
        return;
    }

    LOG_DEBUG << "Connecting to node " << endpoint;
    _connector.connect(
      endpoint,
      base::config::NET_CONNECT_TIMEOUT,
      [this, on_connect](std::unique_ptr<net::Connection> connection) {
          ASSERT(connection);
          auto session = std::make_unique<net::Session>(std::move(connection));
          auto peer = lk::Peer::connected(std::move(session), *this, _core);
          if (_connected_peers.tryAddPeer(peer)) {
              on_connect(peer);
          }
          else {
              on_connect(nullptr);
          }
      },
      [on_connect](const net::Connector::ConnectError&) {
          // TODO: remember, call another callback
          on_connect(nullptr);
      });
    LOG_DEBUG << "Connection to " << endpoint << " is added to queue";
}


PeerTable& Host::getPool() noexcept
{
    return _connected_peers;
}


const PeerTable& Host::getPool() const noexcept
{
    return _connected_peers;
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

    bootstrap();
}


void Host::bootstrap()
{
    if (_config.hasKey("nodes")) {
        for (const auto& node : _config.getVector<std::string>("nodes")) {
            checkOutPeer(net::Endpoint(node), [this](std::shared_ptr<Peer> peer) {
                if (peer) {
                    peer->lookup(_core.getThisNodeAddress(),
                                 base::config::NET_LOOKUP_ALPHA,
                                 [this, peer](std::vector<PeerBase::Info> peers_info) {
                                     peer->startSession();
                                     for (const auto& peer_info : peers_info) {
                                         checkOutPeer(peer_info.endpoint, [this](std::shared_ptr<Peer> peer){
                                             peer->lookup(_core.getThisNodeAddress(), base::config::NET_LOOKUP_ALPHA,
                                                     [this, peer](std::vector<PeerBase::Info> peers_info) {
                                                 for(const auto& peer_info : peers_info) {
                                                     checkOutPeer(peer_info.endpoint, [this](std::shared_ptr<Peer> peer) {
                                                         peer->startSession();
                                                     });
                                                 }
                                             });
                                             peer->startSession();
                                         });
                                     }
                                 });
                }
            });
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
    _connected_peers.removeSilent();
}


void Host::broadcast(const base::Bytes& data)
{
    LOG_DEBUG << "Broadcasting data size = " << data.size();
    _connected_peers.forEachPeer([data](PeerBase& peer) { peer.send(data); });
}


void Host::broadcast(const lk::Block& block)
{
    _connected_peers.forEachPeer([block](PeerBase& peer) { static_cast<lk::Peer&>(peer).sendBlock(block); });
}


void Host::broadcast(const lk::Transaction& tx)
{
    _connected_peers.forEachPeer([tx](PeerBase& peer) { static_cast<lk::Peer&>(peer).sendTransaction(tx); });
}


bool Host::isConnectedTo(const net::Endpoint& endpoint) const
{
    bool have_peer_with_endpoint = false;
    _connected_peers.forEachPeer([&have_peer_with_endpoint, endpoint](const PeerBase& peer) {
        if (peer.getPublicEndpoint() == endpoint) {
            have_peer_with_endpoint = true;
        }
    });
    return have_peer_with_endpoint;
}


std::vector<Peer::Info> Host::allConnectedPeersInfo() const
{
    return _connected_peers.allPeersInfo();
}


unsigned short Host::getPublicPort() const noexcept
{
    return _server_public_port;
}

} // namespace net
