#include "block.hpp"

#include "base/hash.hpp"

#include <utility>

namespace bc
{

Block::Block(bc::BlockDepth depth, base::Sha256 prev_block_hash, TransactionsSet txs)
    : _depth{depth}, _prev_block_hash{std::move(prev_block_hash)}, _txs(std::move(txs))
{}


base::SerializationOArchive& Block::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_depth);
    oa.serialize(_nonce);
    oa.serialize(_prev_block_hash);
    oa.serialize(_txs);
    return oa;
}


Block Block::deserialize(base::SerializationIArchive& ia)
{
    BlockDepth depth = ia.deserialize<BlockDepth>();

    NonceInt nonce = ia.deserialize<NonceInt>();

    auto prev_block_hash = base::Sha256::deserialize(ia);

    TransactionsSet txs = ia.deserialize<TransactionsSet>();

    Block ret{depth, std::move(prev_block_hash), std::move(txs)};
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
        a.getPrevBlockHash() == b.getPrevBlockHash() && a.getTransactions() == b.getTransactions();
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