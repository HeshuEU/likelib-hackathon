#pragma once

#include "net/connection.hpp"

#include <memory>

namespace net
{

class Handler
{
  public:
    //===================
    virtual void onReceive(const base::Bytes& bytes) = 0;
    // virtual void onSend() = 0;
    virtual void onClose() = 0;
    //===================
    virtual ~Handler() = default;
    //===================
};


class Session
{
  public:
    //==================
    using SessionManager = std::function<void(Session& session, const base::Bytes& data)>;
    //==================
    explicit Session(std::unique_ptr<Connection> connection);
    ~Session();
    //==================
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool isClosed() const;
    //==================
    [[nodiscard]] std::size_t getId() const;
    //==================
    void setHandler(std::unique_ptr<Handler> handler);
    //==================
    void send(const base::Bytes& data);
    void send(base::Bytes&& data);
    //==================
    void start();
    void close();
    //==================
    const Endpoint& getEndpoint() const;
    //==================
  private:
    //==================
    std::size_t _id;
    void setNextId();
    //==================
    std::unique_ptr<Connection> _connection;
    std::unique_ptr<Handler> _handler;
    //==================
    void receive();
    //==================
};


} // namespace net