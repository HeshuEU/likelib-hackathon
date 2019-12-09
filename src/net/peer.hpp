#pragma once

#include "base/time.hpp"
#include "net/connection.hpp"
#include "net/packet.hpp"

#include <forward_list>
#include <memory>
#include <shared_mutex>

namespace net
{


class Peer
{
  public:
    //=================
    using PacketHandler = std::function<void(Packet&&)>;
    explicit Peer(std::unique_ptr<net::Connection> connection);
    //=================
    [[nodiscard]] bool isActive() const noexcept;
    [[nodiscard]] bool isClosed() const noexcept;
    //=================
    void sendDataPacket(const base::Bytes& data);
    void sendDataPacket(base::Bytes&& data);
    //=================
    [[nodiscard]] base::Time getLastSeen() const noexcept;
    //=================
    void ping();
    void close();
    //=================
  private:
    //=================
    std::unique_ptr<net::Connection> _connection;
    //=================
    void onConnectionReceived(Packet&& packet);
    PacketHandler _handler;
    //=================
    void onHandshake();
    //=================
    void onPing();
    void onPong();
    //=================
    void refreshLastSeen();
    base::Time _last_seen;
    //=================
};


class Peers
{
  public:
    //=================
    Peers() = default;
    ~Peers() = default;
    //=================
    void add(std::shared_ptr<Peer> peer);
    std::size_t size() const noexcept;
    //=================
    void forEach(std::function<void(Peer&)> f);
    //=================
  private:
    std::forward_list<std::shared_ptr<Peer>> _peers;
    std::size_t _size{0};
    mutable std::shared_mutex _state_mutex;
};


} // namespace net