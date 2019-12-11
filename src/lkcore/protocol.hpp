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
};

} // namespace lk