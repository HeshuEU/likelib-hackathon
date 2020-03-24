#include "host.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include <boost/asio/error.hpp>

#include <chrono>

namespace ba = boost::asio;

namespace lk
{

PeerTable::PeerTable(bc::Address host_id)
  : _host_id{ std::move(host_id) }
{}


Host::Host(const base::PropertyTree& config, std::size_t connections_limit)
  : _config{ config }
  , _listen_ip{ _config.get<std::string>("net.listen_addr") }
  , _server_public_port{ _config.get<unsigned short>("net.public_port") }
  , _max_connections_number{ connections_limit }
  , _acceptor{ _io_context, _listen_ip }
  , _connector{ _io_context }
  , _heartbeat_timer{ _io_context }
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


net::Session& Host::addNewSession(std::unique_ptr<net::Connection> connection)
{
    ASSERT(connection);
    std::unique_lock lk(_connected_peers_mutex);
    _connected_peers.push_back(std::make_shared<net::Session>(std::move(connection)));
    auto& session = *_sessions.back();

    // setHandler won't miss any messages since it is called inside connections callback
    session.setHandler(_handler_factory->create(session));
    return session;
}


void Host::accept()
{
    _acceptor.accept([this](std::unique_ptr<net::Connection> connection) {
        ASSERT(connection);
        [[maybe_unused]] auto& session = addNewSession(std::move(connection));
        accept();
    });
}


void Host::connect(const net::Endpoint& endpoint)
{
    if (_listen_ip == endpoint) {
        return;
    }

    if (isConnectedTo(endpoint)) {
        return;
    }

    std::shared_lock lk(_connected_peers_mutex);
    for (const auto& peer : _connected_peers) {
        if (auto ep = peer.getPublicEndpoint(); ep && *ep == endpoint) {
            return;
        }
    }

    LOG_CURRENT_FUNCTION;
    LOG_DEBUG << "Connecting to node " << endpoint;
    _connector.connect(endpoint, [this](std::unique_ptr<net::Connection> connection) {
        ASSERT(connection);
        [[maybe_unused]] auto& session = addNewSession(std::move(connection));
    });
    LOG_DEBUG << "Connection to " << endpoint << " is added to queue";
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
            connect(net::Endpoint(node));
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
    _connected_peers.erase(std::remove_if(_connected_peers.begin(),
                                          _connected_peers.end(),
                                          [](const auto& peer) { return peer->isClosed(); }),
                           _connected_peers.end());
}


void Host::broadcast(const base::Bytes& data)
{
    std::unique_lock lk(_connected_peers_mutex);
    LOG_DEBUG << "Broadcasting data size = " << data.size();
    for (auto& peer : _connected_peers) {
        peer.send(data);
    }
}


bool Host::isConnectedTo(const net::Endpoint& endpoint) const
{
    std::shared_lock lk(_connected_peers_mutex);
    for (const auto& peer : _connected_peers) {
        if (peer.getEndpoint() == endpoint) {
            return true;
        }
    }
    return false;
}

} // namespace net
