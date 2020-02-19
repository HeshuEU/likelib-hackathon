#pragma once

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include "base/serialization.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"
#include "bc/types.hpp"

#include <iosfwd>

namespace bc
{

class Block
{
  public:
    //=================
    Block(bc::BlockDepth depth, base::Sha256 prev_block_hash, TransactionsSet txs);

    Block(const Block&) = default;
    Block(Block&&) = default;

    Block& operator=(const Block&) = default;
    Block& operator=(Block&&) = default;

    ~Block() = default;
    //=================
    void serialize(base::SerializationOArchive& oa) const;
    [[nodiscard]] static Block deserialize(base::SerializationIArchive& ia);
    //=================
    [[nodiscard]] BlockDepth getDepth() const noexcept;
    [[nodiscard]] const base::Sha256& getPrevBlockHash() const;
    [[nodiscard]] const TransactionsSet& getTransactions() const;
    [[nodiscard]] NonceInt getNonce() const noexcept;
    //=================
    void setDepth(BlockDepth depth) noexcept;
    void setNonce(NonceInt nonce) noexcept;
    void setPrevBlockHash(const base::Sha256& prev_block_hash);
    void setTransactions(TransactionsSet txs);
    void addTransaction(const Transaction& tx);
    //=================
  private:
    //=================
    bc::BlockDepth _depth;
    NonceInt _nonce;

    base::Sha256 _prev_block_hash;
    TransactionsSet _txs;
    //=================
};

std::ostream& operator<<(std::ostream& os, const Block& block);

base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Block& block);
base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Block& block);

bool operator==(const bc::Block& a, const bc::Block& b);
bool operator!=(const bc::Block& a, const bc::Block& b);

} // namespace bc