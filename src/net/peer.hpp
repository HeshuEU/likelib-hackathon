#pragma once

#include "base/time.hpp"
#include "net/connection.hpp"
#include "net/packet.hpp"

#include <memory>

namespace net
{


class Peer
{
  public:
    //=================
    using PacketHandler = std::function<void(Packet&&)>;
    explicit Peer(std::shared_ptr<net::Connection> connection, PacketHandler handler);
    //=================
    [[nodiscard]] bool isActive() const noexcept;
    [[nodiscard]] bool isClosed() const noexcept;
    //=================
    void sendDataPacket(const base::Bytes& data);
    void sendDataPacket(base::Bytes&& data);
    //=================
    [[nodiscard]] base::Time getLastSeen() const noexcept;
    //=================
    void close();
    //=================
  private:
    //=================
    std::shared_ptr<net::Connection> _connection;
    //=================
    void onConnectionReceived(Packet&& packet);
    PacketHandler _handler;
    //=================
    void onHandshake();
    //=================
    void ping();
    void onPing();
    void onPong();
    //=================
    void refreshLastSeen();
    base::Time _last_seen;
    //=================
};


} // namespace net