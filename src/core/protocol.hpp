#pragma once

#include "core/block.hpp"
#include "core/peer.hpp"
#include "core/transaction.hpp"
#include "net/session.hpp"

namespace lk
{

class Core;
class Host;
class Peer;


class Protocol : public ProtocolBase
{
  public:
    //===============
    struct Context
    {
        Core* core;
        PeerPoolBase* pool;
        Host* host;
        Peer* peer;
    };

    /*
     * Creates Protocol object, meaning that we gonna start our session.
     * All members of context must outlive the Protocol object.
     */

    static Protocol peerConnected(Context context);
    static Protocol peerAccepted(Context context);
    //===============
    Protocol(Protocol&&) = default;
    Protocol& operator=(Protocol&&) = delete;
    //===============
    void onReceive(const base::Bytes& bytes) override;
    void onClose() override;
    //===============
    void sendBlock(const lk::Block& block) override;
    void sendTransaction(const lk::Transaction& tx) override;
    void sendSessionEnd(std::function<void()> on_send) override;
    //===============
  private:
    Context _ctx;

    explicit Protocol(Context context);

    void startOnConnectedPeer();
    void startOnAcceptedPeer();
};


// class Network
//{
//    void onNewBlock(const lk::Block& block);
//    void onNewPendingTransaction(const lk::Transaction& tx);
//};

} // namespace core
