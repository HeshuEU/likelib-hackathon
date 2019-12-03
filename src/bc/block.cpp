#include "block.hpp"

#include "base/hash.hpp"

#include <utility>

namespace bc
{

Block::Block(const base::Bytes& prev_block_hash, TransactionsSet&& txs)
    : _prev_block_hash{prev_block_hash}, _txs(std::move(txs))
{}


Block::Block(base::Bytes&& prev_block_hash, TransactionsSet&& txs)
    : _prev_block_hash{std::move(prev_block_hash)}, _txs(std::move(txs))
{}


const base::Bytes& Block::getPrevBlockHash() const
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


void Block::setNonce(NonceInt nonce) noexcept
{
    _nonce = nonce;
}


bool Block::checkValidness() const
{
    //    static const base::Bytes MAX_HASH_VALUE = getComplexity();
    //    base::SerializationOArchive oa;
    //    oa << *this;
    //    return base::Sha256::compute(oa.getBytes()).getBytes() < MAX_HASH_VALUE;
    // TODO: implement
    return true;
}


void Block::setPrevBlockHash(const base::Bytes& prev_block_hash)
{
    _prev_block_hash = prev_block_hash;
}


void Block::setTransactions(TransactionsSet&& txs)
{
    _txs = std::move(txs);
}


void Block::addTransaction(const Transaction& tx)
{
    _txs.add(tx);
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Block& block)
{
    NonceInt nonce;
    ia >> nonce;
    block.setNonce(nonce);

    base::Bytes prev_block_hash;
    ia >> prev_block_hash;
    block.setPrevBlockHash(prev_block_hash);

    TransactionsSet txs;
    ia >> txs;
    block.setTransactions(std::move(txs));

    return ia;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Block& block)
{
    return oa << block.getNonce() << block.getPrevBlockHash() << block.getTransactions();
}

} // namespace bc