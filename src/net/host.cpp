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

Host::Host(const base::PropertyTree& config, DataHandler handler)
    : _listen_ip{config.get<std::string>("net.listen_addr")},
      _server_public_port{config.get<unsigned short>("net.public_port")}, _heartbeat_timer{_io_context},
      _data_handler(handler)
{
    ASSERT(_data_handler);
}


Host::~Host()
{
    _io_context.stop();
    waitForFinish();
}


void Host::run()
{
    acceptClients();
    scheduleHeartBeat();
    _network_thread = std::make_unique<std::thread>(&Host::networkThreadWorkerFunction, this);
}


void Host::scheduleHeartBeat()
{
    std::shared_lock lk(_connections_mutex);
    for(const auto& peer: _peers) {
        // peer->ping();
    }

    _heartbeat_timer.expires_after(std::chrono::seconds(base::config::NET_PING_FREQUENCY));
    _heartbeat_timer.async_wait([this](const boost::system::error_code& ec) {
        dropZombieConnections();
        scheduleHeartBeat();
    });
}


void Host::acceptClients()
{
    ASSERT(!_acceptor);

    using namespace ba::ip;
    _acceptor = std::make_unique<tcp::acceptor>(_io_context, static_cast<ba::ip::tcp::endpoint>(_listen_ip));
    _acceptor->set_option(ba::socket_base::reuse_address(true));

    LOG_INFO << "Listening on " << _listen_ip.toString();
    acceptLoop();
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


void Host::acceptLoop()
{
    _acceptor->async_accept([this](const boost::system::error_code& ec, ba::ip::tcp::socket socket) {
        if(ec) {
            LOG_WARNING << "Connection accept failed: " << ec;
        }
        else {
            auto connection = std::make_shared<Connection>(_io_context, std::move(socket));
            auto peer_on_data_handler = std::bind(&Host::onConnectionReceivedPacketHandler, this, std::placeholders::_1);
            _peers.emplace_back(std::move(connection), std::move(peer_on_data_handler));
            LOG_INFO << "Connection accepted: " << connection->getEndpoint();
        }
        acceptLoop();
    });
}


void Host::connect(const std::vector<net::Endpoint>& addresses)
{
    for(const auto& address: addresses) {
        connect(address);
    }
}


void Host::connect(const net::Endpoint& address)
{
    LOG_DEBUG << "Connecting to " << address.toString();
    auto socket = std::make_unique<ba::ip::tcp::socket>(_io_context);
    socket->async_connect(static_cast<ba::ip::tcp::endpoint>(address),
        [this, socket = std::move(socket), address](const boost::system::error_code& ec) mutable {
            if(ec) {
                switch(ec.value()) {
                    case ba::error::connection_refused: {
                        LOG_WARNING << "Connection error: host " << address.toString();
                        break;
                    }
                    case ba::error::fault: {
                        LOG_WARNING << "Connection error: invalid address";
                        break;
                    }
                    default: {
                        LOG_WARNING << "Connection error: " << ec << ' ' << ec.message();
                        break;
                    }
                }
            }
            else {
                auto connection = std::make_shared<Connection>(_io_context, std::move(*socket.release()),
                    std::bind(
                        &Host::onConnectionReceivedPacketHandler, this, std::placeholders::_1));
                LOG_INFO << "Connection established: " << connection->getEndpoint();

                // connection->setServerEndpoint(address);
                connection->startReceivingMessages();
                net::Packet packet{net::PacketType::HANDSHAKE};
                packet.setPublicServerPort(_server_public_port);
                connection->send(packet);
                _peers.push_back(std::move(connection));
            }
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
    for(auto it = _peers.begin(); it != _peers.end();) {
        auto& peer = *it;
        if(base::Time::now().seconds() - peer.getLastSeen().seconds() > base::config::NET_PING_FREQUENCY) {
            if(!peer.isClosed()) {
                peer.close();
            }
            it = _peers.erase(it);
        }
        else {
            ++it;
        }
    }
}


void Host::onConnectionReceivedPacketHandler(net::Packet&& packet)
{}


void Host::broadcastDataPackage(const base::Bytes& data)
{
    Packet p;
    p.setType(PacketType::DATA);
    p.setData(data);

    for(auto& peer: _peers) {
        peer.sendDataPackage(data);
    }
}

} // namespace net
