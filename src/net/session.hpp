#pragma once

#include "net/peer.hpp"

#include <memory>

namespace net
{


class Session
{
  public:
    //==================
    using SessionManager = std::function<void(Session& session, const base::Bytes& data)>;
    //==================
    explicit Session(std::unique_ptr<Peer> peer);
    //==================
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool isClosed() const;
    //==================
    [[nodiscard]] Id getId() const;
    //==================
    void send(const base::Bytes& data);
    void send(base::Bytes&& data);
    //==================
    void start(SessionManager receive_handler);
    void stop();
    //==================
  private:
    //==================
    std::unique_ptr<Peer> _peer;
    SessionManager _receive_handler;
    //==================
    void receive();
    //==================
};


} // namespace net