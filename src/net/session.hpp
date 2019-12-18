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
    Session(std::unique_ptr<Connection> connection, std::unique_ptr<Handler> handler);
    ~Session();
    //==================
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool isClosed() const;
    //==================
    [[nodiscard]] std::size_t getId() const;
    //==================
    void send(const base::Bytes& data);
    void send(base::Bytes&& data);
    //==================
    void close();
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