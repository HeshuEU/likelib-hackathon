#pragma once

#include "base/bytes.hpp"
#include "bc/transaction.hpp"

#include <vector>

namespace bc
{

class Block
{
  public:
    Block(const base::Bytes& prev_block_hash, const std::vector<bc::Transaction>& txs);
    Block(base::Bytes&& prev_block_hash, std::vector<bc::Transaction>&& txs);

  private:
    base::Bytes _prev_block_hash;
    std::vector<bc::Transaction> _txs;
};


class BlockBuilder
{
  public:
    void setPrevHash(const base::Bytes& prev_block_hash);

    void setTransactions(const std::vector<bc::Transaction>& txs);
    void setTransactions(std::vector<bc::Transaction>&& txs);

    void addTransaction(const bc::Transaction& tx);
    void addTransaction(bc::Transaction&& tx);

    Block build() const&;
    Block build() &&;

  private:
    base::Bytes _prev_block_hash;
    std::vector<bc::Transaction> _txs;
};

} // namespace bc