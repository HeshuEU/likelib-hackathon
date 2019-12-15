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
    explicit Peer(std::unique_ptr<net::Connection> connection);
    //=================
    [[nodiscard]] bool isActive() const noexcept;
    [[nodiscard]] bool isClosed() const noexcept;
    //=================
    using ReceiveHandler = std::function<void(const base::Bytes&)>;
    void receive(ReceiveHandler handler);
    //=================
    void send(base::Bytes data);
    //=================
    [[nodiscard]] base::Time getLastSeen() const noexcept;
    //=================
    void close();
    //=================
    std::size_t getId() const noexcept;
    //=================
  private:
    //=================
    const std::size_t _id;
    std::size_t getNextId() const;
    //==================
    std::shared_ptr<net::Connection> _connection;
    //=================
    void refreshLastSeen();
    base::Time _last_seen;
    //=================
};

bool operator<(const Peer& a, const Peer& b);
bool operator>(const Peer& a, const Peer& b);
bool operator<=(const Peer& a, const Peer& b);
bool operator>=(const Peer& a, const Peer& b);
bool operator==(const Peer& a, const Peer& b);
bool operator!=(const Peer& a, const Peer& b);


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
    void removeClosed();
    //=================
  private:
    std::forward_list<std::shared_ptr<Peer>> _peers;
    std::size_t _size{0};
    mutable std::shared_mutex _state_mutex;
};

} // namespace net