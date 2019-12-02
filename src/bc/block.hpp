#pragma once

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include "base/serialization.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"
#include "bc/types.hpp"

namespace bc
{

class Block
{
  public:
    //=================
    Block() = default;
    Block(const base::Bytes& prev_block_hash, TransactionsSet&& txs);
    Block(base::Bytes&& prev_block_hash, TransactionsSet&& txs);

    Block(const Block&) = default;
    Block(Block&&) = default;

    Block& operator=(const Block&) = default;
    Block& operator=(Block&&) = default;

    ~Block() = default;
    //=================
    const base::Bytes& getPrevBlockHash() const;
    const TransactionsSet& getTransactions() const;
    NonceInt getNonce() const noexcept;
    //=================
    void setNonce(NonceInt nonce) noexcept;
    void setPrevBlockHash(const base::Bytes& prev_block_hash);
    void setTransactions(TransactionsSet&& txs);
    void addTransaction(const Transaction& tx);
    //=================



    //=================

    bool checkValidness() const;

  private:
    NonceInt _nonce;

    base::Bytes _prev_block_hash;
    TransactionsSet _txs;
};


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Block& block);
base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Block& block);

} // namespace bc