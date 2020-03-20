#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "net/acceptor.hpp"
#include "net/connector.hpp"
#include "net/session.hpp"

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
    //=================================
    class HandlerFactory
    {
      public:
        //===================
        virtual std::unique_ptr<Session::Handler> create(Session& session) = 0;
        virtual void destroy() = 0;
        //===================
        virtual ~HandlerFactory() = default;
        //===================
    };
    //=================================
    explicit Host(const base::PropertyTree& config, std::size_t connections_limit);
    ~Host();
    //=================================
    void connect(const Endpoint& address);
    bool isConnectedTo(const Endpoint& endpoint) const;
    //=================================
    void broadcast(const base::Bytes& data);
    //=================================
    void run(std::unique_ptr<HandlerFactory> handler_factory);
    void join();
    //=================================
  private:
    //=================================
    const base::PropertyTree& _config;
    //=================================
    const Endpoint _listen_ip;
    const unsigned short _server_public_port;
    //=================================
    boost::asio::io_context _io_context;
    //===================
    std::thread _network_thread;
    void networkThreadWorkerFunction() noexcept;
    //=================================
    std::vector<std::shared_ptr<Session>> _sessions;
    mutable std::shared_mutex _sessions_mutex;

    net::Acceptor _acceptor;
    net::Connector _connector;

    std::unique_ptr<HandlerFactory> _handler_factory;

    void accept();
    Session& addNewSession(std::unique_ptr<Connection> peer);
    //=================================
    boost::asio::steady_timer _heartbeat_timer;
    void scheduleHeartBeat();
    void dropZombieConnections();
    //=================================
};

} // namespace net