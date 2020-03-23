#pragma once

#include "lk/peer.hpp"
#include "net/session.hpp"

namespace lk
{

//class Core;
//class Network;
//class Peer;


class Protocol : public net::Session::Handler
{
  public:
    Protocol(Peer& peer);

    void onReceive(const base::Bytes& bytes) override;
    void onClose() override;
  private:
    Peer& _peer;
};



class Network
{
    void onNewBlock(const bc::Block& block);
    void onNewPendingTransaction(const bc::Transaction& tx);
};

} // namespace lk
