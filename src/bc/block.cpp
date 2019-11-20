#include "block.hpp"

#include <utility>

namespace bc
{

Block::Block(const base::Bytes& prev_block_hash, std::vector<bc::Transaction>&& txs)
    : _prev_block_hash{prev_block_hash}, _txs(std::move(txs))
{}


Block::Block(base::Bytes&& prev_block_hash, std::vector<bc::Transaction>&& txs)
    : _prev_block_hash{std::move(prev_block_hash)}, _txs(std::move(txs))
{}


const base::Bytes& Block::getPrevBlockHash() const
{
    return _prev_block_hash;
}


const std::vector<bc::Transaction>& Block::getTransactions() const
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
    static const base::Bytes MAX_HASH_VALUE{0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    return serialize() < MAX_HASH_VALUE;
}


void Block::setPrevBlockHash(const base::Bytes& prev_block_hash)
{
    _prev_block_hash = prev_block_hash;
}


void Block::setTransactions(std::vector<Transaction>&& txs)
{
    _txs = std::move(txs);
}


base::SerializationIArchive operator>>(base::SerializationIArchive& ia, Block& block)
{
    NonceInt nonce;
    ia >> nonce;
    block.setNonce(nonce);

    base::Bytes prev_block_hash;
    ia >> prev_block_hash;
    block.setPrevBlockHash(prev_block_hash);

    std::vector<Transaction> txs;
    ia >> txs;
    block.setTransactions(std::move(txs));

    return ia;
}


base::SerializationOArchive operator<<(base::SerializationOArchive& oa, const Block& block)
{
    return oa << block.getNonce() << block.getPrevBlockHash() << block.getTransactions();
}

} // namespace bc