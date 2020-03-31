#pragma once

#include "base/utility.hpp"
#include "core/block.hpp"
#include "core/peer.hpp"
#include "core/transaction.hpp"
#include "net/session.hpp"

namespace lk
{

// clang-format off
DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(MessageType, std::uint8_t,
                                          (NOT_AVAILABLE)
                                            (CANNOT_ACCEPT)
                                            (ACCEPTED)
                                            (ACCEPTED_RESPONSE)
                                            (PING)
                                            (PONG)
                                            (TRANSACTION)
                                            (GET_BLOCK)
                                            (BLOCK)
                                            (BLOCK_NOT_FOUND)
                                            (GET_INFO)
                                            (INFO)
                                            (NEW_NODE)
                                            (CLOSE)
)
// clang-format on

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

    struct State
    {
        MessageType message_we_are_waiting_for{MessageType::NOT_AVAILABLE};
        MessageType last_processed{MessageType::NOT_AVAILABLE};
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
    State& getState() noexcept;
    const State& getState() const noexcept;
    //===============
  private:
    Context _ctx;
    State _state;

    explicit Protocol(Context context);

    void startOnConnectedPeer();
    void startOnAcceptedPeer();
};


//========================================

class CannotAcceptMessage
{
  public:
    DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(RefusionReason, std::uint8_t,
                                              (NOT_AVAILABLE)
                                                (BUCKET_IS_FULL)
                                                (BAD_RATING))

    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, RefusionReason why_not_accepted);
    static CannotAcceptMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    RefusionReason _why_not_accepted;
    std::vector<lk::PeerBase::Info> _peers_info;

    CannotAcceptMessage(RefusionReason why_not_accepted, std::vector<lk::PeerBase::Info> peers_info);
};

//========================================

class AcceptedMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const lk::Block& top_block,
                          const lk::Address& address,
                          std::uint16_t public_port,
                          const std::vector<lk::Peer::Info>& known_peers);
    static AcceptedMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& context, Protocol& protocol);

  private:
    lk::Block _theirs_top_block;
    lk::Address _address;
    std::uint16_t
      _public_port; // zero public port states that peer didn't provide information about his public endpoint
    std::vector<lk::Peer::Info> _known_peers;

    AcceptedMessage(lk::Block&& top_block,
                    lk::Address address,
                    std::uint16_t public_port,
                    std::vector<lk::Peer::Info>&& known_peers);
};

//========================================

class AcceptedResponseMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const lk::Block& top_block,
                          const lk::Address& address,
                          std::uint16_t public_port,
                          const std::vector<lk::Peer::Info>& known_peers);
    static AcceptedResponseMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& context, Protocol& protocol);

  private:
    lk::Block _theirs_top_block;
    lk::Address _address;
    std::uint16_t
      _public_port; // zero public port states that peer didn't provide information about his public endpoint
    std::vector<lk::Peer::Info> _known_peers;

    AcceptedResponseMessage(lk::Block&& top_block,
                            lk::Address address,
                            std::uint16_t public_port,
                            std::vector<lk::Peer::Info>&& known_peers);
};

//========================================

class PingMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PingMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    PingMessage() = default;
};

//============================================

class PongMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PongMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    PongMessage() = default;
};

//============================================

class TransactionMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const lk::Transaction& tx);
    static TransactionMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    lk::Transaction _tx;

    TransactionMessage(const lk::Transaction& tx);
};

//============================================

class GetBlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    static GetBlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    base::Sha256 _block_hash;

    GetBlockMessage(base::Sha256 block_hash);
};

//============================================

class BlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const lk::Block& block);
    static BlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    lk::Block _block;

    BlockMessage(lk::Block block);
};

//============================================

class BlockNotFoundMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    static BlockNotFoundMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    base::Sha256 _block_hash;

    BlockNotFoundMessage(base::Sha256 block_hash);
};

//============================================

class GetInfoMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static GetInfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    GetInfoMessage() = default;
};

//============================================

class InfoMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const base::Sha256& top_block_hash,
                          const std::vector<net::Endpoint>& available_peers);
    static InfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    base::Sha256 _top_block_hash;
    std::vector<net::Endpoint> _available_peers;

    InfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers);
};

//============================================

class NewNodeMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa,
                          const net::Endpoint& new_node_endpoint,
                          const lk::Address& address);
    static NewNodeMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    net::Endpoint _new_node_endpoint;
    lk::Address _address;

    NewNodeMessage(net::Endpoint&& new_node_endpoint, lk::Address&& address);
};

//============================================

class CloseMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static CloseMessage deserialize(base::SerializationIArchive& ia);
    void handle(const lk::Protocol::Context& ctx, Protocol& protocol);

  private:
    CloseMessage();
};

//============================================

} // namespace lk
