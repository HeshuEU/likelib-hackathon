#include "block.hpp"

#include "base/hash.hpp"

#include <utility>

namespace bc
{

Block::Block(bc::BlockDepth depth,
             base::Sha256 prev_block_hash,
             base::Time timestamp,
             bc::Address coinbase,
             TransactionsSet txs)
  : _depth{ depth }
  , _prev_block_hash{ std::move(prev_block_hash) }
  , _timestamp{ std::move(timestamp) }
  , _coinbase{ std::move(coinbase) }
  , _txs(std::move(txs))
{}


void Block::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_depth);
    oa.serialize(_nonce);
    oa.serialize(_prev_block_hash);
    oa.serialize(_timestamp);
    oa.serialize(_coinbase);
    oa.serialize(_txs);
}


Block Block::deserialize(base::SerializationIArchive& ia)
{
    auto depth = ia.deserialize<BlockDepth>();
    auto nonce = ia.deserialize<NonceInt>();
    auto prev_block_hash = ia.deserialize<base::Sha256>();
    auto timestamp = ia.deserialize<base::Time>();
    auto coinbase = ia.deserialize<bc::Address>();
    auto txs = ia.deserialize<TransactionsSet>();
    Block ret{ depth, std::move(prev_block_hash), std::move(timestamp), std::move(coinbase), std::move(txs) };
    ret.setNonce(nonce);
    return ret;
}


bc::BlockDepth Block::getDepth() const noexcept
{
    return _depth;
}


const base::Sha256& Block::getPrevBlockHash() const
{
    return _prev_block_hash;
}


const TransactionsSet& Block::getTransactions() const
{
    return _txs;
}


NonceInt Block::getNonce() const noexcept
{
    return _nonce;
}


void Block::setDepth(BlockDepth depth) noexcept
{
    _depth = depth;
}


void Block::setNonce(NonceInt nonce) noexcept
{
    _nonce = nonce;
}


const base::Time& Block::getTimestamp() const noexcept
{
    return _timestamp;
}


const bc::Address& Block::getCoinbase() const noexcept
{
    return _coinbase;
}


void Block::setPrevBlockHash(const base::Sha256& prev_block_hash)
{
    _prev_block_hash = prev_block_hash;
}


void Block::setTransactions(TransactionsSet txs)
{
    _txs = std::move(txs);
}


void Block::addTransaction(const Transaction& tx)
{
    _txs.add(tx);
}


bool operator==(const bc::Block& a, const bc::Block& b)
{
    return a.getDepth() == b.getDepth() && a.getNonce() == b.getNonce() &&
           a.getPrevBlockHash() == b.getPrevBlockHash() && a.getTimestamp() == b.getTimestamp() &&
           a.getCoinbase() == b.getCoinbase() && a.getTransactions() == b.getTransactions();
}


bool operator!=(const bc::Block& a, const bc::Block& b)
{
    return !(a == b);
}


std::ostream& operator<<(std::ostream& os, const Block& block)
{
    return os << base::Sha256::compute(base::toBytes(block));
}

} // namespace bc
