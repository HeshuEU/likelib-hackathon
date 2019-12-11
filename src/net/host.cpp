#include "host.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "bc/blockchain.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/error.hpp>

#include <chrono>

namespace ba = boost::asio;

namespace net
{

Host::Host(const base::PropertyTree& config)
    : _listen_ip{config.get<std::string>("net.listen_addr")}, _server_public_port{config.get<unsigned short>(
                                                                  "net.public_port")},
      _acceptor{_io_context, _listen_ip, _peers}, _connector{_io_context, _peers}, _heartbeat_timer{_io_context}
{}


Host::~Host()
{
    _io_context.stop();
    waitForFinish();
}


void Host::run(std::shared_ptr<HostHandler> handler)
{
    ASSERT(handler);

    _handler = std::move(handler);
    acceptClients();
    scheduleHeartBeat();
    _network_thread = std::make_unique<std::thread>(&Host::networkThreadWorkerFunction, this);
}


void Host::scheduleHeartBeat()
{
    _heartbeat_timer.expires_after(std::chrono::seconds(base::config::NET_PING_FREQUENCY));
    _heartbeat_timer.async_wait([this](const boost::system::error_code& ec) {
        dropZombieConnections();
        scheduleHeartBeat();
    });
}


void Host::acceptClients()
{
    _acceptor.acceptInfinite([this](Peer& peer) {
        _handler->onAccept(peer);
    });
}


void Host::networkThreadWorkerFunction() noexcept
{
    try {
        _io_context.run();
    }
    catch(...) {
        // global catch done for safety, since thread function cannot throw.
        // TODO: thread worker function error-handling
    }
}


void Host::connect(const std::vector<net::Endpoint>& addresses)
{
    for(const auto& address: addresses) {
        connect(address);
    }
}


void Host::connect(const net::Endpoint& address)
{
    _connector.connect(address, [this](Peer& peer) {
        _handler->onConnect(peer);
    });
}



void Host::waitForFinish()
{
    if(_network_thread && _network_thread->joinable()) {
        _network_thread->join();
        _network_thread.reset(nullptr);
    }
}


void Host::dropZombieConnections()
{
    _peers.forEach([this](Peer& peer) {
        if(base::Time::now().seconds() - peer.getLastSeen().seconds() > base::config::NET_PING_FREQUENCY) {
            if(!peer.isClosed()) {
                peer.close();
            }
        }
    });
}


void Host::broadcast(const base::Bytes& data)
{
    _peers.forEach([&data](Peer& peer) {
        peer.send(data);
    });
}

} // namespace net
