#pragma once

#include "base/bytes.hpp"
#include "base/crypto.hpp"
#include "base/hash.hpp"
#include "base/utility.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "bc/transaction.hpp"
#include "net/host.hpp"

#include <forward_list>

namespace lk
{

//================

// clang-format off
    DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(MessageType, std::uint8_t,
                                              (NOT_AVAILABLE)
                                                      (HANDSHAKE)
                                                      (PING)
                                                      (PONG)
                                                      (TRANSACTION)
                                                      (GET_BLOCK)
                                                      (BLOCK)
                                                      (BLOCK_NOT_FOUND)
                                                      (GET_INFO)
                                                      (INFO)
                                                      (NEW_NODE)
    )
// clang-format on


class Core;
class Network;
class Peer;


struct PeerInfo
{
    net::Endpoint endpoint;
    bc::Address address;

    static PeerInfo deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
};


class HandshakeMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const bc::Block& block, const bc::Address& address,
        std::uint16_t public_port, const std::vector<PeerInfo>& known_peers);
    void serialize(base::SerializationOArchive& oa) const;
    static HandshakeMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Block _theirs_top_block;
    bc::Address _address;
    std::uint16_t
        _public_port; // zero public port states that peer didn't provide information about his public endpoint
    std::vector<PeerInfo> _known_peers;

    HandshakeMessage(
        bc::Block&& top_block, bc::Address address, std::uint16_t public_port, std::vector<PeerInfo>&& known_peers);
};


class PingMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PingMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    PingMessage() = default;
};

//============================================

class PongMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PongMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    PongMessage() = default;
};

//============================================

class TransactionMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, bc::Transaction tx);
    void serialize(base::SerializationOArchive& oa) const;
    static TransactionMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Transaction _tx;

    TransactionMessage(bc::Transaction tx);
};

//============================================

class GetBlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    void serialize(base::SerializationOArchive& oa) const;
    static GetBlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _block_hash;

    GetBlockMessage(base::Sha256 block_hash);
};

//============================================

class BlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const bc::Block& block);
    void serialize(base::SerializationOArchive& oa) const;
    static BlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Block _block;

    BlockMessage(bc::Block block);
};

//============================================

class BlockNotFoundMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    void serialize(base::SerializationOArchive& oa) const;
    static BlockNotFoundMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _block_hash;

    BlockNotFoundMessage(base::Sha256 block_hash);
};

//============================================

class GetInfoMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static GetInfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    GetInfoMessage() = default;
};

//============================================

class InfoMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& top_block_hash,
        const std::vector<net::Endpoint>& available_peers);
    void serialize(base::SerializationOArchive& oa) const;
    static InfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _top_block_hash;
    std::vector<net::Endpoint> _available_peers;

    InfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers);
};

//============================================

class NewNodeMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(
        base::SerializationOArchive& oa, const net::Endpoint& new_node_endpoint, const bc::Address& address);
    void serialize(base::SerializationOArchive& oa) const;
    static NewNodeMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    net::Endpoint _new_node_endpoint;
    bc::Address _address;

    NewNodeMessage(net::Endpoint&& new_node_endpoint, bc::Address&& address);
};

//============================================

class MessageProcessor
{
  public:
    MessageProcessor(Peer& peer, Network& network, Core& core);

    void process(const base::Bytes& raw_message);

  private:
    static const base::TypeList<HandshakeMessage, PingMessage, PongMessage, TransactionMessage, GetBlockMessage,
        BlockMessage, BlockNotFoundMessage, GetInfoMessage, InfoMessage, NewNodeMessage>
        _all_message_types;

    Peer& _peer;
    Network& _network;
    Core& _core;
};


//================

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
    Peer(Network& owning_network_object, net::Session& session, Core& _core);
    //================
    net::Endpoint getEndpoint() const;
    std::optional<net::Endpoint> getPublicEndpoint() const;
    void setServerEndpoint(net::Endpoint endpoint);
    //================
    std::optional<bc::Address> getAddress() const;
    void setAddress(bc::Address address);
    //================
    void setState(State new_state);
    State getState() const noexcept;
    //================
    void addSyncBlock(bc::Block block);
    bool applySyncs();
    const std::forward_list<bc::Block>& getSyncBlocks() const noexcept;
    //================
    [[nodiscard]] std::unique_ptr<net::Handler> createHandler();
    //================
    void send(const base::Bytes& data);
    void send(base::Bytes&& data);
    //================
  private:
    //================
    class Handler : public net::Handler
    {
      public:
        //===================
        Handler(Peer& owning_peer, Network& owning_network_object, net::Session& handled_session, Core& core);
        ~Handler() override = default;
        //================
        void onReceive(const base::Bytes& data) override;
        // virtual void onSend() = 0;
        void onClose() override;
        //================
      private:
        //================
        Peer& _owning_peer;
        Network& _owning_network_object;
        net::Session& _session;
        Core& _core;
        //================
        MessageProcessor _message_processor;
        //================
    };
    //================
    void doHandshake();
    //================
    Network& _owning_network_object;
    net::Session& _session;
    Core& _core;
    //================
    State _state{State::JUST_ESTABLISHED};
    std::optional<net::Endpoint> _endpoint_for_incoming_connections;
    std::optional<bc::Address> _address;
    //================
    std::forward_list<bc::Block> _sync_blocks;
    //================
};


class Network
{
  public:
    //================
    Network(const base::PropertyTree& config, Core& core);
    //================
    void run();
    //================
    [[nodiscard]] std::vector<PeerInfo> allConnectedPeersInfo() const;
    bool checkOutNode(const net::Endpoint& endpoint, const bc::Address& address);
    //================
    void broadcast(const base::Bytes& data);
    //================
  private:
    //================
    friend class Peer; // in order to be able to call removePeer from Handler
    //================
    class HandlerFactory : public net::Host::HandlerFactory
    {
      public:
        //================
        explicit HandlerFactory(Network& owning_network_object);
        ~HandlerFactory() override = default;
        //================
        std::unique_ptr<net::Handler> create(net::Session& session) override;
        void destroy() override;
        //================
      private:
        //================
        Network& _owning_network_object;
        //================
    };
    //================
    const base::PropertyTree& _config;
    net::Host _host;
    std::forward_list<Peer> _peers;
    Core& _core;
    //================
    std::optional<std::uint16_t> _public_port;
    //================
    Peer& createPeer(net::Session& session);
    void removePeer(const Peer& peer);
    //================
    void onNewBlock(const bc::Block& block);
    void onNewPendingTransaction(const bc::Transaction& tx);
    //================
};

} // namespace lk
