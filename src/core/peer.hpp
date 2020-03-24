#pragma once

#include "base/time.hpp"
#include "core/address.hpp"
#include "core/block.hpp"
#include "core/protocol.hpp"
#include "net/session.hpp"

#include <forward_list>
#include <memory>

namespace lk
{

class Core;
class Host;

class Peer
{
  public:
    //================
    enum class State
    {
        JUST_ESTABLISHED,
        REQUESTED_BLOCKS,
        SYNCHRONISED
    };
    //================
    struct Info
    {
        net::Endpoint endpoint;
        lk::Address address;

        static Info deserialize(base::SerializationIArchive& ia);
        void serialize(base::SerializationOArchive& oa) const;
    };
    //================
    static std::unique_ptr<Peer> accepted(std::unique_ptr<net::Session> session, lk::Core& core, lk::Host& host);
    static std::unique_ptr<Peer> connected(std::unique_ptr<net::Session> session, lk::Core& core, lk::Host& host);
    //================
    base::Time getLastSeen() const;
    net::Endpoint getEndpoint() const;
    std::optional<net::Endpoint> getPublicEndpoint() const;
    void setServerEndpoint(net::Endpoint endpoint);

    void setProtocol(std::unique_ptr<lk::Protocol> protocol);
    //================
    const lk::Address& getAddress() const noexcept;
    void setAddress(lk::Address address);
    //================
    void setState(State new_state);
    State getState() const noexcept;

    Info getInfo() const;
    bool isClosed() const;
    //================
    void addSyncBlock(lk::Block block);
    void applySyncs();
    const std::forward_list<lk::Block>& getSyncBlocks() const noexcept;
    //================
    void send(const base::Bytes& data);
    void send(base::Bytes&& data);
    //================
  private:
    //================
    Peer(std::unique_ptr<net::Session> session, lk::Core& core);
    //================
    std::unique_ptr<net::Session> _session;
    //================
    State _state{ State::JUST_ESTABLISHED };
    std::optional<net::Endpoint> _endpoint_for_incoming_connections;
    lk::Address _address;
    //================
    std::forward_list<lk::Block> _sync_blocks;
    //================
    lk::Core& _core;
    std::unique_ptr<lk::Protocol> _protocol;
    //================
};

}