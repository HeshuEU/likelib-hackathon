#pragma once

#include "base/bytes.hpp"
#include "bc/transaction.hpp"

#include <vector>

namespace bc
{

class Block
{
  public:
    //=================
    Block(const base::Bytes& prev_block_hash, std::vector<bc::Transaction>&& txs);
    Block(base::Bytes&& prev_block_hash, std::vector<bc::Transaction>&& txs);

    Block(const Block&) = delete; // to avoid having 2 equal blocks
    Block(Block&&) = default;

    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = default;

    ~Block() = default;
    //=================

    const base::Bytes& getPrevBlockHash() const;
    const std::vector<bc::Transaction>& getTransactions() const;

    base::Bytes serialize() const;

  private:
    base::Bytes _prev_block_hash;
    std::vector<bc::Transaction> _txs;
};


class BlockBuilder
{
  public:
    void setPrevHash(const base::Bytes& prev_block_hash);

    void setTransactions(std::vector<bc::Transaction>&& txs);

    void addTransaction(bc::Transaction&& tx);

    Block build() &&;

  private:
    base::Bytes _prev_block_hash;
    std::vector<bc::Transaction> _txs;
};

} // namespace bc