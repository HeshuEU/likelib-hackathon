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
    Block(base::Sha256 prev_block_hash, TransactionsSet txs);

    Block(const Block&) = default;
    Block(Block&&) = default;

    Block& operator=(const Block&) = default;
    Block& operator=(Block&&) = default;

    ~Block() = default;
    //=================
    static base::SerializationOArchive& serialize(base::SerializationOArchive& oa, const Block& block);
    static Block deserialize(base::SerializationIArchive& ia);
    //=================
    const base::Sha256& getPrevBlockHash() const;
    const TransactionsSet& getTransactions() const;
    NonceInt getNonce() const noexcept;
    //=================
    void setNonce(NonceInt nonce) noexcept;
    void setPrevBlockHash(const base::Sha256& prev_block_hash);
    void setTransactions(TransactionsSet txs);
    void addTransaction(const Transaction& tx);
    //=================



    //=================
  private:
    //=================
    NonceInt _nonce;

    base::Sha256 _prev_block_hash;
    TransactionsSet _txs;
    //=================
};


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Block& block);
base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Block& block);

} // namespace bc