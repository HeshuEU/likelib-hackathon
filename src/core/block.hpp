#pragma once

#include "core/transaction.hpp"
#include "core/transactions_set.hpp"
#include "core/types.hpp"

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include "base/serialization.hpp"

#include <optional>
#include <variant>

namespace lk
{

class ImmutableBlock
{
  public:
    //=================
    ImmutableBlock(BlockDepth depth,
                   NonceInt nonce,
                   base::Sha256 prev_block_hash,
                   base::Time timestamp,
                   Address coinbase,
                   TransactionsSet txs);

    ImmutableBlock(const ImmutableBlock&) = default;
    ImmutableBlock(ImmutableBlock&&) = default;

    ImmutableBlock& operator=(const ImmutableBlock&) = delete;
    ImmutableBlock& operator=(ImmutableBlock&&) = delete;

    ~ImmutableBlock() = default;
    //=================
    void serialize(base::SerializationOArchive& oa) const;
    [[nodiscard]] static ImmutableBlock deserialize(base::SerializationIArchive& ia);
    //=================
    BlockDepth getDepth() const noexcept;
    const base::Sha256& getPrevBlockHash() const noexcept;
    const TransactionsSet& getTransactions() const noexcept;
    NonceInt getNonce() const noexcept;
    const base::Time& getTimestamp() const noexcept;
    const Address& getCoinbase() const noexcept;
    //=================
    const base::Sha256& getHash() const noexcept;
    //=================
  private:
    //=================
    const BlockDepth _depth;
    const NonceInt _nonce;
    const base::Sha256 _prev_block_hash;
    const base::Time _timestamp;
    const Address _coinbase;
    const TransactionsSet _txs;
    //=================
    const base::Sha256 _this_block_hash;

    base::Sha256 computeThisBlockHash();
    //=================
};


class MutableBlock
{
  public:
    MutableBlock(BlockDepth depth,
                 NonceInt nonce,
                 base::Sha256 prev_block_hash,
                 base::Time timestamp,
                 Address coinbase,
                 TransactionsSet txs);

    MutableBlock(const MutableBlock&) = default;
    MutableBlock(MutableBlock&&) = default;

    MutableBlock& operator=(const MutableBlock&) = default;
    MutableBlock& operator=(MutableBlock&&) = default;

    ~MutableBlock() = default;
    //=================
    void serialize(base::SerializationOArchive& oa) const;
    [[nodiscard]] static MutableBlock deserialize(base::SerializationIArchive& ia);
    //=================
    BlockDepth getDepth() const noexcept;
    NonceInt getNonce() const noexcept;
    const base::Sha256& getPrevBlockHash() const noexcept;
    const TransactionsSet& getTransactions() const noexcept;
    const base::Time& getTimestamp() const noexcept;
    const Address& getCoinbase() const noexcept;
    //=================
    void setDepth(BlockDepth depth) noexcept;
    void setNonce(NonceInt nonce) noexcept;
    void setPrevBlockHash(const base::Sha256& prev_block_hash);
    void setTimestamp(base::Time timestamp);
    void setTransactions(TransactionsSet txs);
    void addTransaction(const Transaction& tx);

  private:
    //=================
    BlockDepth _depth;
    NonceInt _nonce;
    base::Sha256 _prev_block_hash;
    base::Time _timestamp;
    Address _coinbase;
    TransactionsSet _txs;
    //=================
};

bool operator==(const ImmutableBlock& a, const ImmutableBlock& b);
bool operator!=(const ImmutableBlock& a, const ImmutableBlock& b);

bool operator==(const MutableBlock& a, const MutableBlock& b);
bool operator!=(const MutableBlock& a, const MutableBlock& b);


class BlockFieldsView
{
  public:
    //=================
    BlockFieldsView(const ImmutableBlock& block);
    BlockFieldsView(const MutableBlock& block);
    //=================
    BlockDepth getDepth() const noexcept;
    const base::Sha256& getPrevBlockHash() const noexcept;
    const TransactionsSet& getTransactions() const noexcept;
    NonceInt getNonce() const noexcept;
    const base::Time& getTimestamp() const noexcept;
    const lk::Address& getCoinbase() const noexcept;
    //=================
  private:
    const ImmutableBlock* _b1;
    const MutableBlock* _b2;
};


class BlockBuilder
{
  public:
    BlockBuilder() = default;
    explicit BlockBuilder(const ImmutableBlock& block);
    explicit BlockBuilder(const MutableBlock& b);

    void setDepth(BlockDepth depth);
    void setNonce(NonceInt nonce);
    void setPrevBlockHash(base::Sha256 hash);
    void setTimestamp(base::Time timestamp);
    void setCoinbase(Address address);
    void setTransactionsSet(TransactionsSet txs);

    ImmutableBlock buildImmutable() const&;
    MutableBlock buildMutable() const&;

    ImmutableBlock buildImmutable() &&;
    MutableBlock buildMutable() &&;

  private:
    std::optional<BlockDepth> _depth;
    std::optional<NonceInt> _nonce;
    std::optional<base::Sha256> _prev_block_hash;
    std::optional<base::Time> _timestamp;
    std::optional<Address> _coinbase;
    std::optional<TransactionsSet> _txs;

    template<typename T>
    void initFromBlock(const T& b);

    void raiseIfNotEverythingIsSet() const;
    void null();
};

} // namespace lk

#include "block.tpp"
