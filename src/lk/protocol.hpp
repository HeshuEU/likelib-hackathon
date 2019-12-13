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

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(MessageType, unsigned char, (TRANSACTION)(BLOCK)(GET_BLOCK))

class MessageHandlerRouter
{
  public:
    //==================
    MessageHandlerRouter(bc::Blockchain& blockchain, net::Host& host, net::Peer& peer);
    //==================
    void handle(const base::Bytes& data);
    //==================
  private:
    //==================
    bc::Blockchain& _blockchain;
    net::Host& _host;
    net::Peer& _peer;
    //==================
    void onTransaction(bc::Transaction&& tx);
    void onBlock(bc::Block&& block);
    void onGetBlock(base::Sha256&& block_hash);
    //==================
};


class ProtocolEngine
{
  public:
    ProtocolEngine(const base::PropertyTree& config, bc::Blockchain& blockchain);
    void run();

    void broadcastBlock(const bc::Block& block);
    void broadcastTransaction(const bc::Transaction& tx);

    boost::signals2::signal<void(const bc::Transaction&)> signal_transaction_received;

  private:
    const base::PropertyTree& _config;
    net::Host _host;
    bc::Blockchain& _blockchain;
    std::map<std::size_t, MessageHandlerRouter> _routers;

    void onAccept(net::Peer& peer);
    void onConnect(net::Peer& peer);
    void onReceive(net::Peer& peer, const base::Bytes& data);

    void acceptLoop();
    void connectToPeersFromConfig();
};


} // namespace lk