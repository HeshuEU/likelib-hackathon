#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "node/miner.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"

#include <boost/signals2.hpp>

#include <shared_mutex>
#include <unordered_map>

namespace bc
{

class Blockchain
{
  public:
    //===================
    Blockchain();
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    ~Blockchain() = default;
    //===================
    bool tryAddBlock(const Block& block);
    std::optional<bc::Block> findBlock(const base::Sha256& block_hash) const;
    //===================
    boost::signals2::signal<void(const Block&)> signal_block_added;
    //===================
  private:
    //===================
    std::unordered_map<base::Sha256, Block> _blocks;
    base::Sha256 _top_level_block_hash;
    mutable std::shared_mutex _blocks_mutex;
    //===================
    void setupGenesis();
    //===================
};

} // namespace bc