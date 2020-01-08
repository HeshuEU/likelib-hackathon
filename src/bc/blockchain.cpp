#include "blockchain.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "net/host.hpp"

#include <optional>

namespace bc
{

Blockchain::Blockchain(const base::PropertyTree& config)
    : _config{config}, _top_level_block_hash(base::Bytes(32)), _database{_config}
{}


void Blockchain::load()
{
    for(const auto& block_hash: _database.createAllBlockHashesList()) {
        LOG_DEBUG << "Loading block " << block_hash << " from database";
        tryAddBlock(_database.getBlock(block_hash));
    }
}


void Blockchain::addGenesisBlock(const Block& block)
{
    auto hash = base::Sha256::compute(base::toBytes(block));

    std::lock_guard lk(_blocks_mutex);
    if(!_blocks.empty()) {
        RAISE_ERROR(base::LogicError, "cannot add genesis to non-empty chain");
    }

    auto inserted_block = _blocks.insert({hash, block}).first;
    _database.addBlock(hash, block);
    _top_level_block_hash = hash;

    LOG_DEBUG << "Adding genesis block. Block hash = " << hash;
    signal_block_added(inserted_block->second);
}



bool Blockchain::tryAddBlock(const Block& block)
{
    auto hash = base::Sha256::compute(base::toBytes(block));
    decltype(_blocks)::iterator inserted_block;
    {
        std::lock_guard lk(_blocks_mutex);
        if(!_blocks.empty() && _blocks.find(hash) != _blocks.end()) {
            return false;
        }
        else if(!_blocks.empty() && _top_level_block_hash != block.getPrevBlockHash()) {
            return false;
        }
        else if(_blocks.size() - 1 != block.getDepth()) {
            return false;
        }
        else {
            inserted_block = _blocks.insert({hash, block}).first;
            _database.addBlock(hash, block);
            _top_level_block_hash = hash;
        }
    }

    LOG_DEBUG << "Adding block. Block hash = " << hash;
    signal_block_added(inserted_block->second);

    return true;
}


std::optional<Block> Blockchain::findBlock(const base::Sha256& block_hash) const
{
    std::shared_lock lk(_blocks_mutex);
    if(auto it = _blocks.find(block_hash); it != _blocks.end()) {
        return it->second;
    }
    else {
        return std::nullopt;
    }
}


std::optional<bc::Transaction> Blockchain::findTransaction(const base::Sha256& tx_hash) const
{
    std::shared_lock lk(_blocks_mutex);
    for(const auto& block: _blocks) {
        for(const auto& tx: block.second.getTransactions()) {
            if(base::Sha256::compute(base::toBytes(tx)) == tx_hash) {
                return tx;
            }
        }
    }
    return std::nullopt;
}


const Block& Blockchain::getTopBlock() const
{
    std::shared_lock lk(_blocks_mutex);
    auto it = _blocks.find(_top_level_block_hash);
    ASSERT(it != _blocks.end());
    return it->second;
}

} // namespace bc
