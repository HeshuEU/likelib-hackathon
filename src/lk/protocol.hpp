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

class Peer
{
  public:
    //================
    class Handler : public net::Handler
    {
    public:
        //===================
        explicit Handler(net::Session& handled_session);
        ~Handler() override = default;
        //================
        void onReceive(const base::Bytes& bytes) override;
        // virtual void onSend() = 0;
        void onClose() override;
        //================
    private:
        net::Session& _handled_session;
    };
    //================
  private:
    //================

    //================
};


class Network
{
  public:
    //================
    Network(const base::PropertyTree& config, Core& core);

    class HandlerFactory : public net::HandlerFactory
    {
      public:
        //================
        explicit HandlerFactory(Network& owning_network_object);
        ~HandlerFactory() override = default;
        //================
        std::unique_ptr<net::Handler> create() override;
        void destroy() override;
        void close();
        //================
    private:
        //================
        Network& _owning_network_object;
        //================
    };
    //================
    void run();
    //================
    void broadcastBlock(const bc::Block& block);
    void broadcastTransaction(const bc::Transaction& tx);
    //================
  private:
    net::Host _host;
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