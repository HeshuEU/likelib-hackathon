#pragma once

#include "base/database.hpp"
#include "base/property_tree.hpp"
#include "base/utility.hpp"

#include "core/block.hpp"
#include "core/consensus.hpp"
#include "core/transaction.hpp"
#include "core/transactions_set.hpp"

#include <shared_mutex>
#include <unordered_map>

namespace lk
{

class Blockchain
{
  public:
    //===================
    Blockchain(lk::Block genesis_block, const base::PropertyTree& config);
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    ~Blockchain() = default;
    //===================
    void load();
    //===================
    void addGenesisBlock(const Block& block);
    //===================
    enum class AdditionResult
    {
        ADDED,
        ALREADY_IN_BLOCKCHAIN,
        INVALID_PARENT_HASH,
        INVALID_DEPTH,
        OLD_TIMESTAMP,
        FUTURE_TIMESTAMP,
        INVALID_TRANSACTIONS_NUMBER,
        INVALID_TRANSACTIONS,
        CONSENSUS_ERROR,
    };

    AdditionResult tryAddBlock(const Block& block);
    //===================
    std::optional<base::Sha256> findBlockHashByDepth(BlockDepth depth) const;
    std::optional<Block> findBlock(const base::Sha256& block_hash) const;
    std::optional<Transaction> findTransaction(const base::Sha256& tx_hash) const;

    std::vector<std::optional<std::reference_wrapper<const Block>>> getByDepth(
      const std::vector<BlockDepth>& depths) const;
    std::vector<std::optional<std::reference_wrapper<const Block>>> getByDepthFromTop(
      const std::vector<BlockDepth>& depths) const;
    //===================
    std::pair<Block, Complexity> getTopBlockAndComplexity() const;
    Block getTopBlock() const;
    base::Sha256 getTopBlockHash() const;

    // *getNthFromTop(0) == getTopBlock()
    std::optional<std::reference_wrapper<const Block>> getNthFromTop(BlockDepth depth_from_top) const;
    //===================
  private:
    //===================
    const base::PropertyTree& _config;
    //===================
    std::unordered_map<base::Sha256, const Block> _blocks;
    std::map<lk::BlockDepth, base::Sha256> _blocks_by_depth;
    base::Sha256 _top_level_block_hash;
    mutable std::shared_mutex _blocks_mutex;

    /*
     * Thread-unsafe: prefix _ means that its purpose it the same as getTopBlock,
     * but it is an unsafe version. Used in getTopBlock(), only with lock.
     */
    const Block& _getTopBlock() const;
    //===================
    base::Database _database;
    mutable std::shared_mutex _database_rw_mutex;
    //===================
    Consensus _consensus;
    bool checkConsensus(const Block& block) const;
    //===================
    base::Observable<Block> _block_added;
    //===================
    void pushForwardToPersistentStorage(const base::Sha256& block_hash, const lk::Block& block);
    std::optional<base::Sha256> getLastBlockHashAtPersistentStorage() const;
    std::optional<lk::Block> findBlockAtPersistentStorage(const base::Sha256& block_hash) const;
    std::vector<base::Sha256> createAllBlockHashesListAtPersistentStorage() const;
    //===================
};

} // namespace lk
