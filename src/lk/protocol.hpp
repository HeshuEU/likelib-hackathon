#pragma once

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include "base/stringifiable_enum_class.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "bc/transaction.hpp"
#include "net/host.hpp"

#include <forward_list>

namespace lk
{

class Core;
class Network;


class Peer
{
  public:
    //================
    Peer(Network& owning_network_object, net::Session& session, Core& _core);
    //================
    [[nodiscard]] std::optional<net::Endpoint> getServerEndpoint() const;
    void setServerEndpoint(net::Endpoint endpoint);
    //================
    [[nodiscard]] std::unique_ptr<net::Handler> createHandler();
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
        std::forward_list<bc::Block> _sync_blocks;
        //================
        void onHandshakeMessage(bc::Block&& top_block_hash, std::optional<std::uint16_t>&& public_port);
        void onPingMessage();
        void onPongMessage();
        void onTransactionMessage(bc::Transaction&& tx);
        void onBlockMessage(bc::Block&& block);
        void onGetBlockMessage(base::Sha256&& block_hash);
        void onGetInfoMessage();
        void onInfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers);
        //================
    };
    //================
    enum class State
    {
        CONNECTED,
        ACCEPTED,
        REQUESTED_BLOCKS,
        SYNCHRONISED
    };
    //================
    void doHandshake();
    //================
    Network& _owning_network_object;
    net::Session& _session;
    Core& _core;
    //================
    State _state{State::SYNCHRONISED};
    std::optional<net::Endpoint> _address_for_incoming_connections;
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
    void broadcastBlock(const bc::Block& block);
    void broadcastTransaction(const bc::Transaction& tx);
    //================
    [[nodiscard]] std::vector<net::Endpoint> allPeersAddresses() const;
    //================
  private:
    //================
    friend class Peer; // in order to be able to call removePeer from Handler
    //================
    class HandlerFactory : public net::HandlerFactory
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
    std::vector<Peer> _peers;
    Core& _core;
    //================
    std::optional<std::uint16_t> _public_port;
    //================
    Peer& createPeer(net::Session& session);
    void removePeer(const Peer& peer);
    //================
};

/*
class SessionManager
{
  public:
    //================
    static std::unique_ptr<SessionManager> accepted(net::Session& session, Core& core, net::Host& host);
    static std::unique_ptr<SessionManager> connected(net::Session& session, Core& core, net::Host& host);
    //================
    void handle(net::Session& session, const base::Bytes& data);
    //================
  private:
    //================
    enum class State
    {
        CONNECTED,
        ACCEPTED,
        REQUESTED_BLOCKS,
        SYNCHRONISED
    };
    State _state;

    std::optional<base::Sha256> _top_block_hash;
    std::stack<bc::Block> _sync_blocks_stack;
    //================
    SessionManager(Core& core, net::Host& host);
    //================
    Core& _core;
    net::Host& _host;
    //================
    void onPing(net::Session& session);
    void onPong(net::Session& session);
    void onTransaction(net::Session& session, bc::Transaction&& tx);
    void onBlock(net::Session& session, bc::Block&& block);
    void onGetBlock(net::Session& session, base::Sha256&& hash);
    void onInfo(net::Session& session);
    //================
    void handshakeRoutine(net::Session& session);
    void onHandshake(net::Session& session, base::Sha256&& top_block_hash);
    void onSyncBlock(net::Session& session, bc::Block&& block);
    //================
};


class Network
{
  public:
    Network(const base::PropertyTree& config, Core& core);

    void run();

    void broadcastBlock(const bc::Block& block);
    void broadcastTransaction(const bc::Transaction& tx);

  private:
    const base::PropertyTree& _config;
    Core& _core;
    net::Host _host;
    std::unordered_map<net::Id, std::unique_ptr<SessionManager>> _peers;
};
 */

} // namespace lk