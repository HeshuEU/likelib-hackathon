#pragma once

#include "net/session.hpp"

namespace lk
{

class Core;
class Host;
class Peer;


class MessageProcessor
{
  public:
    //===============
    struct Context
    {
        Core* core;
        Host* host;
        Peer* peer;
    };
    //===============
    MessageProcessor(Context context);

    /*
     * Decode and act according to received data.
     * Throws if invalid message, or there is an error during handling.
     */
    void process(const base::Bytes& raw_message);

  private:
    const Context _ctx;
};


class Protocol : public net::Session::Handler
{
  public:
    //===============
    /*
     * Creates Protocol object, meaning that we gonna start our session.
     * All members of context must outlive the Protocol object.
     */

    static Protocol peerConnected(MessageProcessor::Context context);
    static Protocol peerAccepted(MessageProcessor::Context context);
    //===============
    Protocol(Protocol&&) = default;
    Protocol& operator=(Protocol&&) = delete;
    //===============
    void onReceive(const base::Bytes& bytes) override;

    /*
     * Called once when peer is closed.
     */
    void onClose() override;
    //===============
  private:
    MessageProcessor::Context _ctx;
    MessageProcessor _processor;

    explicit Protocol(MessageProcessor::Context context);

    void startOnConnectedPeer();
    void startOnAcceptedPeer();
    void doHandshake();
};


//class Network
//{
//    void onNewBlock(const lk::Block& block);
//    void onNewPendingTransaction(const lk::Transaction& tx);
//};

} // namespace core
