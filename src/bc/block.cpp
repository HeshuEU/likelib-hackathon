#include "block.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <sstream>
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


base::Bytes Block::serialize() const
{
    std::ostringstream oss;
    boost::archive::text_oarchive to(oss);
    to << _prev_block_hash << _txs.size();

    std::size_t index = 0;
    for(const auto& tx : _txs) {
        to << tx.serialize().toVector();
    }

    return base::Bytes(oss.str());
}


void BlockBuilder::setTransactions(std::vector<bc::Transaction>&& txs)
{
    _txs = std::move(txs);
}


void BlockBuilder::addTransaction(bc::Transaction&& tx)
{
    _txs.push_back(std::move(tx));
}


Block BlockBuilder::build() &&
{
    return Block(std::move(_prev_block_hash), std::move(_txs));
}

} // namespace bc