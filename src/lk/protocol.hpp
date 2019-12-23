#pragma once

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include "base/stringifiable_enum_class.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "bc/transaction.hpp"
#include "net/host.hpp"


namespace lk
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    MessageType, unsigned char, (NOT_AVAILABLE)(HANDSHAKE)(PING)(PONG)(TRANSACTION)(BLOCK)(GET_BLOCK)(INFO))

class Core;
class Network;
class Peer;


namespace mh
{
    class IHandler
    {
      public:
        virtual ~IHandler() = default;

        virtual bool canHandle(MessageType type) const = 0;

        struct Data
        {
            Peer& peer;
            Network& network;
            Core& core;
        };
        virtual void handle(MessageType type, Data& data, base::Bytes& bytes) = 0;
    };


    class Ping : public IHandler
    {
      public:
        bool canHandle(MessageType type) const override;
        void handle(MessageType type, Data& data, base::Bytes& bytes) override;
    };


    class Pong : public IHandler
    {
      public:
        bool canHandle(MessageType type) const override;
        void handle(MessageType type, Data& data, base::Bytes& bytes) override;
    };


    class Transaction : public IHandler
    {
      public:
        bool canHandle(MessageType type) const override;
        void handle(MessageType type, Data& data, base::Bytes& bytes) override;
    };

    class AllHandler final : public IHandler
    {
      public:
        AllHandler();
        bool canHandle(MessageType type) const override;
        void handle(MessageType type, Data& data, base::Bytes& bytes) override;

      private:
        std::vector<std::unique_ptr<IHandler>> _handlers;
    };
} // namespace mh


class Peer
{
  public:
    //================
    Peer(Network& owning_network_object, net::Session& session);
    //================
    enum class State
    {
        CONNECTED,
        ACCEPTED,
        REQUESTED_BLOCKS,
        SYNCHRONISED
    };
    //================
  private:
    class Handler;

  public:
    std::unique_ptr<Handler> createHandler();
    //================
  private:
    //================
    class Handler : public net::Handler
    {
      public:
        //===================
        Handler(Peer& owning_peer, Network& owning_network_object, net::Session& handled_session);
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
        //================
        void onPing();
        void onPong();
        void onTransaction(bc::Transaction&& tx);
        void onBlock(bc::Block&& block);
        void onGetBlock(base::Sha256&& block_hash);
        void onInfo();
        //================
    };
    //================
    Network& _owning_network_object;
    net::Session& _session;
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
    net::Host _host;
    mh::AllHandler _all_message_handlers;
    std::vector<Peer> _peers;
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