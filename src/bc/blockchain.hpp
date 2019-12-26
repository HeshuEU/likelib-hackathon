#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "bc/database_manager.hpp"
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
    Blockchain(const base::PropertyTree& config);
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    ~Blockchain() = default;
    //===================
    void load();
    //===================
    void addGenesisBlock(const Block& block);
    bool tryAddBlock(const Block& block);
    std::optional<bc::Block> findBlock(const base::Sha256& block_hash) const;
    std::optional<bc::Transaction> findTransaction(const base::Sha256& tx_hash) const;
    //===================
    const bc::Block& getTopBlock() const;
    //===================
    boost::signals2::signal<void(const Block&)> signal_block_added;
    //===================
  private:
    //===================
    const base::PropertyTree& _config;
    //===================
    std::unordered_map<base::Sha256, Block> _blocks;
    base::Sha256 _top_level_block_hash;
    mutable std::shared_mutex _blocks_mutex;
    //===================
    bc::DatabaseManager _database;
    //===================
    void setupGenesis();
    //===================
};

} // namespace bc
