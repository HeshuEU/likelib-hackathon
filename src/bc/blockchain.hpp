#pragma once

#include "base/database.hpp"
#include "base/property_tree.hpp"
#include "base/utility.hpp"

#include "bc/block.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"

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
    std::optional<base::Sha256> findBlockHashByDepth(bc::BlockDepth depth) const;
    std::optional<bc::Block> findBlock(const base::Sha256& block_hash) const;
    std::optional<bc::Transaction> findTransaction(const base::Sha256& tx_hash) const;
    //===================
    const bc::Block& getTopBlock() const;
    //===================

    //===================
  private:
    //===================
    const base::PropertyTree& _config;
    bool _is_loaded;
    //===================
    std::unordered_map<base::Sha256, Block> _blocks;
    std::map<bc::BlockDepth, base::Sha256> _blocks_by_depth;
    base::Sha256 _top_level_block_hash;
    mutable std::shared_mutex _blocks_mutex;
    //===================
    base::Database _database;
    mutable std::shared_mutex _database_rw_mutex;
    //===================
    base::Observable<const bc::Block&> _block_added;
    //===================
    void pushForwardToPersistentStorage(const base::Sha256& block_hash, const bc::Block& block);
    std::optional<base::Sha256> getLastBlockHashAtPersistentStorage() const;
    std::optional<bc::Block> findBlockAtPersistentStorage(const base::Sha256& block_hash) const;
    std::vector<base::Sha256> createAllBlockHashesListAtPersistentStorage() const;
    //===================
};

} // namespace bc
