#pragma once

#include "base/time.hpp"
#include "core/address.hpp"
#include "core/block.hpp"
#include "net/session.hpp"

#include <forward_list>
#include <memory>

namespace lk
{

class Core;


class PeerBase : public std::enable_shared_from_this<PeerBase>
{
  public:
    //===========================
    struct Info
    {
        net::Endpoint endpoint;
        lk::Address address;

        static Info deserialize(base::SerializationIArchive& ia);
        void serialize(base::SerializationOArchive& oa) const;
    };
    //===========================
    virtual bool isClosed() const = 0;
    //===========================
    virtual void send(const base::Bytes& data) = 0;
    virtual void send(base::Bytes&& data) = 0;

    virtual const lk::Address& getAddress() const noexcept = 0;
    virtual Info getInfo() const = 0;
    virtual base::Time getLastSeen() const = 0;
    virtual net::Endpoint getEndpoint() const = 0;
    virtual net::Endpoint getPublicEndpoint() const = 0;
    //===========================
};


class PeerPoolBase
{
  public:
    virtual bool tryAddPeer(std::shared_ptr<PeerBase> peer) = 0;
    virtual void removePeer(std::shared_ptr<PeerBase> peer) = 0;

    virtual void forEachPeer(std::function<void(const PeerBase&)> f) const = 0;
    virtual void forEachPeer(std::function<void(PeerBase&)> f) = 0;

    virtual std::vector<PeerBase::Info> allPeersInfo() const = 0;
};


/*
 * Protocol doesn't manage states of session or peer.
 * It just prepares, sends and handles messages.
 */
class ProtocolBase : public net::Session::Handler
{
  public:
    virtual void sendTransaction(const lk::Transaction& tx) = 0;
    virtual void sendBlock(const lk::Block& block) = 0;
    virtual void sendSessionEnd(std::function<void()> on_send) = 0; // TODO: set close reason
};


class Peer : public PeerBase
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
    static std::unique_ptr<Peer> accepted(std::unique_ptr<net::Session> session, lk::PeerPoolBase& host, lk::Core& core);
    static std::unique_ptr<Peer> connected(std::unique_ptr<net::Session> session, lk::PeerPoolBase& host, lk::Core& core);
    //================
    base::Time getLastSeen() const override;
    net::Endpoint getEndpoint() const override;
    net::Endpoint getPublicEndpoint() const override;
    void setServerEndpoint(net::Endpoint endpoint);

    void setProtocol(std::shared_ptr<lk::ProtocolBase> protocol);
    void start();
    //================
    const lk::Address& getAddress() const noexcept override;
    void setAddress(lk::Address address);
    //================
    void setState(State new_state);
    State getState() const noexcept;

    Info getInfo() const override;
    bool isClosed() const;
    //================
    void addSyncBlock(lk::Block block);
    void applySyncs();
    const std::forward_list<lk::Block>& getSyncBlocks() const noexcept;
    //================
    void send(const base::Bytes& data) override;
    void send(base::Bytes&& data) override;

    void send(const lk::Block& block);
    void send(const lk::Transaction& tx);
    //================
  private:
    //================
    Peer(std::unique_ptr<net::Session> session, lk::PeerPoolBase& pool, lk::Core& core);
    //================
    std::unique_ptr<net::Session> _session;
    //================
    State _state{ State::JUST_ESTABLISHED };
    std::optional<net::Endpoint> _endpoint_for_incoming_connections;
    lk::Address _address;
    //================
    std::forward_list<lk::Block> _sync_blocks;
    //================
    bool _is_attached_to_pool{false};
    lk::PeerPoolBase& _pool;
    lk::Core& _core;
    std::shared_ptr<lk::ProtocolBase> _protocol;
    //================
    void rejectedByPool();
    //================
};

}