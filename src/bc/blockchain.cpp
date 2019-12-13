#include "blockchain.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "net/host.hpp"

#include <optional>

namespace bc
{

Blockchain::Blockchain() : _top_level_block_hash(base::Bytes(32))
{
    setupGenesis();
}


bool Blockchain::tryAddBlock(const Block& block)
{
    auto hash = base::Sha256::compute(base::toBytes(block));
    {
        std::lock_guard lk(_blocks_mutex);
        if(!_blocks.empty() && _blocks.find(hash) != _blocks.end()) {
            return false;
        }
        else if(!_blocks.empty() && _top_level_block_hash != block.getPrevBlockHash()) {
            return false;
        }
        else {
            _blocks[hash] = block;
            _top_level_block_hash = std::move(hash);
        }
    }
    LOG_DEBUG << "Adding block. Block hash = " << hash.getBytes().toHex();

    signal_block_added(_blocks[hash]);
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


void Blockchain::setupGenesis()
{
    Block genesis;
    genesis.setNonce(0);
    genesis.setPrevBlockHash(base::Bytes(32));

    static const bc::Address BASE_ADDRESS{std::string(32, '0')};
    static const bc::Balance BASE_MONEY_AMOUNT = 0xffffffff;
    genesis.addTransaction({BASE_ADDRESS, BASE_ADDRESS, BASE_MONEY_AMOUNT, base::Time()});

    auto hash = base::Sha256::compute(base::toBytes(genesis));
    _top_level_block_hash = std::move(hash);
}

} // namespace bc