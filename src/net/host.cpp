#include "host.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/error.hpp>

#include <chrono>

namespace ba = boost::asio;

namespace net
{

Host::Host(const base::PropertyTree& config)
    : _config{config}, _listen_ip{_config.get<std::string>("net.listen_addr")},
      _server_public_port{_config.get<unsigned short>("net.public_port")}, _acceptor{_io_context, _listen_ip},
      _connector{_io_context}, _heartbeat_timer{_io_context}
{}


Host::~Host()
{
    _io_context.stop();
}


void Host::scheduleHeartBeat()
{
    _heartbeat_timer.expires_after(std::chrono::seconds(base::config::NET_PING_FREQUENCY));
    _heartbeat_timer.async_wait([this](const boost::system::error_code& ec) {
        dropZombieConnections();
        scheduleHeartBeat();
    });
}


Session& Host::addNewSession(std::unique_ptr<Connection> connection)
{
    ASSERT(connection);
    std::unique_lock lk(_sessions_mutex);
    _sessions.push_back(std::make_shared<Session>(std::move(connection)));
    auto& session = *_sessions.back();
    session.setHandler(_handler_factory->create(session));
    return session;
}


void Host::accept()
{
    _acceptor.accept([this](std::unique_ptr<Connection> connection) {
        ASSERT(connection);
        auto& session = addNewSession(std::move(connection));
    });
}


void Host::connect(const net::Endpoint& address)
{
    _connector.connect(address, [this](std::unique_ptr<Connection> connection) {
        ASSERT(connection);
        auto& session = addNewSession(std::move(connection));
    });
}


void Host::networkThreadWorkerFunction() noexcept
{
    try {
        _io_context.run();
    }
    catch(const std::exception& e) {
        // TODO: thread worker function error-handling
        LOG_WARNING << boost::stacktrace::stacktrace();
        LOG_WARNING << "Error occurred in network thread: " << e.what();
    }
    catch(...) {
        // global catch done for safety, since thread function cannot throw.
        LOG_WARNING << "Error occurred in network thread";
    }
}


void Host::run(std::unique_ptr<HandlerFactory> handler_factory)
{
    ASSERT(handler_factory);

    _handler_factory = std::move(handler_factory);

    accept();

    if(_config.hasKey("nodes")) {
        for(const auto& node: _config.getVector<std::string>("nodes")) {
            connect(net::Endpoint(node));
        }
    }

    scheduleHeartBeat();

    _network_thread = std::thread(&Host::networkThreadWorkerFunction, this);
}


void Host::join()
{
    if(_network_thread.joinable()) {
        _network_thread.join();
    }
}


void Host::dropZombieConnections()
{
    std::unique_lock lk(_sessions_mutex);
    _sessions.erase(std::remove_if(_sessions.begin(), _sessions.end(),
                        [](auto& session) {
                            return session->isClosed();
                        }),
        _sessions.end());
}


void Host::broadcast(const base::Bytes& data)
{
    std::shared_lock lk(_sessions_mutex);
    for(auto& session: _sessions) {
        session->send(data);
    }
}

} // namespace net
