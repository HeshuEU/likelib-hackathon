#pragma once

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include "base/stringifiable_enum_class.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "bc/transaction.hpp"
#include "net/host.hpp"

namespace bc
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    MessageType, unsigned char, (TRANSACTION)(BLOCK)(GET_BLOCK))

class Message
{
  public:
    //===================
    [[nodiscard]] static Message blockBroadcast(const bc::Block& block);
    [[nodiscard]] static Message transactionBroadcast(const bc::Transaction& tx);
    [[nodiscard]] static Message getBlock(const base::Sha256& hash);
    //===================
    [[nodiscard]] static Message deserialize(const base::Bytes& bytes);
    [[nodiscard]] static Message deserialize(base::SerializationIArchive& bytes);
    //===================
    [[nodiscard]] MessageType getType() const noexcept;
    [[nodiscard]] const base::Bytes& getBytes() const& noexcept;
    [[nodiscard]] base::Bytes&& getBytes() && noexcept;
    //===================
  private:
    //===================
    Message(MessageType type, const base::Bytes& bytes);
    Message(MessageType type, base::Bytes&& bytes);
    //===================
    MessageType _type;
    base::Bytes _raw_data;
    //===================
};

base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Message& v);
base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Message& v);


class MessageHandlerManager
{
  public:
    //================
    void handle(net::Peer& peer, base::Bytes&& bytes);
    //================
    using OnBlockHandler = std::function<void(net::Peer& peer, const Block&)>;
    void addOnBlockMessageHandler(OnBlockHandler f);
    //================
    using OnTransactionHandler = std::function<void(net::Peer& peer, const Transaction&)>;
    void addOnTransactionMessageHandler(OnTransactionHandler f);
    //================
    using OnGetBlockHandler = std::function<void(net::Peer& peer, const base::Sha256&)>;
    void addOnGetBlockMessageHandler(OnGetBlockHandler f);
    //================
  private:
    //================
    std::vector<OnBlockHandler> _on_block_handlers;
    std::vector<OnTransactionHandler> _on_transaction_handlers;
    std::vector<OnGetBlockHandler> _on_get_block_handlers;
    //================
};


class ProtocolEngine
{
  public:
    ProtocolEngine();

    void broadcastTransaction(const bc::Transaction& tx);
    void broadcastBlock(const bc::Block& block);
private:
    MessageHandlerManager _handler_manager;
    net::Host _host;
};


} // namespace bc