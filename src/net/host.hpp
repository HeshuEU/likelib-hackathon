#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "net/acceptor.hpp"
#include "net/connector.hpp"
#include "net/session.hpp"
#include "net/peer.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

#include <list>
#include <memory>
#include <set>
#include <shared_mutex>
#include <thread>

namespace net
{

class Host
{
  public:
    //===================
    Host(const base::PropertyTree& config);
    ~Host();
    //===================
    void connect(const Endpoint& address);
    //===================
    void broadcast(const base::Bytes& data);
    //===================
    using AcceptHandler = std::function<void(Session&)>;
    using ConnectHandler = std::function<void(Session&)>;

    void run(AcceptHandler on_accept, ConnectHandler on_connect, Session::SessionManager receive_handler);
    void join();
    //===================
  private:
    //===================
    const base::PropertyTree& _config;
    //===================
    const Endpoint _listen_ip;
    const unsigned short _server_public_port;
    //===================
    boost::asio::io_context _io_context;
    //===================
    std::thread _network_thread;
    void networkThreadWorkerFunction() noexcept;
    //===================
    std::vector<std::shared_ptr<Session>> _sessions;
    std::shared_mutex _sessions_mutex;

    net::Acceptor _acceptor;
    net::Connector _connector;

    AcceptHandler _accept_handler;
    ConnectHandler _connect_handler;
    Session::SessionManager _receive_handler;

    void accept();
    Session& addNewSession(std::unique_ptr<Peer> peer);
    //===================
    boost::asio::steady_timer _heartbeat_timer;
    void scheduleHeartBeat();
    void dropZombieConnections();
    //===================
};

} // namespace net