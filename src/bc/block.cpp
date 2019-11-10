#include "block.hpp"

#include <utility>

namespace bc
{

Block::Block(const base::Bytes& prev_block_hash, const std::vector<bc::Transaction>& txs)
    : _prev_block_hash{prev_block_hash}, _txs(txs)
{}


Block::Block(base::Bytes&& prev_block_hash, std::vector<bc::Transaction>&& txs)
    : _prev_block_hash{std::move(prev_block_hash)}, _txs(std::move(txs))
{}


void BlockBuilder::setTransactions(const std::vector<bc::Transaction>& txs)
{
    _txs = txs;
}


void BlockBuilder::setTransactions(std::vector<bc::Transaction>&& txs)
{
    _txs = std::move(txs);
}


void BlockBuilder::addTransaction(const bc::Transaction& tx)
{
    _txs.push_back(tx);
}


void BlockBuilder::addTransaction(bc::Transaction&& tx)
{
    _txs.push_back(std::move(tx));
}


Block BlockBuilder::build() const&
{
    return Block(_prev_block_hash, _txs);
}


Block BlockBuilder::build() &&
{
    return Block(std::move(_prev_block_hash), std::move(_txs));
}

} // namespace bc