#pragma once

#include "lk/peer.hpp"
#include "net/session.hpp"

namespace lk
{

class Core;
class Host;
class Peer;


class Protocol : public net::Session::Handler
{
  public:
    //===============
    Protocol(Peer& peer);
    //===============
    void onReceive(const base::Bytes& bytes) override;
    void onClose() override;
    //===============
  private:
    Core& _core;
    Peer& _peer;
    Host& _host;

    void doHandshake();
};


class Network
{
    void onNewBlock(const bc::Block& block);
    void onNewPendingTransaction(const bc::Transaction& tx);
};

} // namespace lk
