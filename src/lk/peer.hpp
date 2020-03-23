#pragma once

#include "bc/address.hpp"
#include "bc/block.hpp"
#include "net/session.hpp"

#include <forward_list>
#include <memory>

namespace lk
{

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
        bc::Address address;

        static Info deserialize(base::SerializationIArchive& ia);
        void serialize(base::SerializationOArchive& oa) const;
    };
    //================
    explicit Peer(std::unique_ptr<net::Session> session);
    //================
    const net::Session& getSession() const noexcept;
    net::Endpoint getEndpoint() const;
    std::optional<net::Endpoint> getPublicEndpoint() const;
    void setServerEndpoint(net::Endpoint endpoint);
    //================
    std::optional<bc::Address> getAddress() const;
    void setAddress(bc::Address address);
    //================
    void setState(State new_state);
    State getState() const noexcept;

    Info getInfo() const;
    //================
    void addSyncBlock(bc::Block block);
    bool applySyncs();
    const std::forward_list<bc::Block>& getSyncBlocks() const noexcept;
    //================
    void send(const base::Bytes& data);
    void send(base::Bytes&& data);
    //================
  private:
    //================
    void doHandshake();
    //================
    std::unique_ptr<net::Session> _session;
    //================
    State _state{ State::JUST_ESTABLISHED };
    std::optional<net::Endpoint> _endpoint_for_incoming_connections;
    std::optional<bc::Address> _address;
    //================
    std::forward_list<bc::Block> _sync_blocks;
    //================
};

}