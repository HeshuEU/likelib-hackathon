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
    MessageType, unsigned char, (SOME_ZERO_ENUM)(PING)(PONG)(TRANSACTION)(BLOCK)(GET_BLOCK))

class Core;


class MessageHandler
{
  public:
    //================
    MessageHandler(Core& core, net::Host& host);
    //================
    void handle(net::Session& session, const base::Bytes& data);
    //================
  private:
    //================
    Core& _core;
    net::Host& _host;
    //================
    void onPing(net::Session& session);
    void onPong(net::Session& session);
    void onTransaction(bc::Transaction&& tx);
    void onBlock(bc::Block&& block);
    void onGetBlock(net::Session& session, base::Sha256&& hash);
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
    net::Host _host;
    MessageHandler _handler;
};

} // namespace lk