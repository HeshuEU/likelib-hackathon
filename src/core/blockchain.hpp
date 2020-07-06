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


class IBlockchain
{
  public:
    //===================
    virtual ~IBlockchain() = default;
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

    virtual AdditionResult tryAddBlock(const ImmutableBlock& block) = 0;
    //===================
    virtual std::optional<ImmutableBlock> findBlock(const base::Sha256& block_hash) const = 0;
    virtual std::optional<base::Sha256> findBlockHashByDepth(BlockDepth depth) const = 0;
    //===================
    virtual ImmutableBlock getGenesisBlock() const = 0;
    virtual ImmutableBlock getTopBlock() const = 0;
    virtual base::Sha256 getTopBlockHash() const = 0;
    virtual std::pair<ImmutableBlock, Complexity> getTopBlockAndComplexity() const = 0;
    //===================
    virtual std::optional<Transaction> findTransaction(const base::Sha256& tx_hash) const = 0;
    //===================
};


class Blockchain : public IBlockchain
{
  public:
    //===================
    Blockchain(ImmutableBlock genesis_block, const base::PropertyTree& config);
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    ~Blockchain() override = default;
    //===================
    AdditionResult tryAddBlock(const ImmutableBlock& block) override;
    //===================
    std::optional<base::Sha256> findBlockHashByDepth(BlockDepth depth) const override;
    std::optional<ImmutableBlock> findBlock(const base::Sha256& block_hash) const override;
    std::optional<Transaction> findTransaction(const base::Sha256& tx_hash) const override;
    //===================
    ImmutableBlock getGenesisBlock() const override;
    std::pair<ImmutableBlock, Complexity> getTopBlockAndComplexity() const override;
    ImmutableBlock getTopBlock() const override;
    base::Sha256 getTopBlockHash() const override;
    //===================
  private:
    //===================
    const base::PropertyTree& _config;
    //===================
    std::unordered_map<base::Sha256, const ImmutableBlock> _blocks;
    std::map<lk::BlockDepth, base::Sha256> _blocks_by_depth;
    base::Sha256 _genesis_block_hash;
    base::Sha256 _top_level_block_hash;
    mutable std::shared_mutex _blocks_mutex;

    void addGenesisBlock(ImmutableBlock block);

    /*
     * Thread-unsafe: prefix _ means that its purpose it the same as getTopBlock,
     * but it is an unsafe version. Used in getTopBlock(), only with lock.
     */
    const ImmutableBlock& _getTopBlock() const;
    //===================
    Consensus _consensus;
    bool checkConsensus(const ImmutableBlock& block) const;
    //===================
    base::Observable<ImmutableBlock> _block_added;
    //===================
};


class PersistentBlockchain : public Blockchain
{
  public:
    //===================
    PersistentBlockchain(ImmutableBlock genesis_block, const base::PropertyTree& config);
    PersistentBlockchain(const Blockchain&) = delete;
    PersistentBlockchain(Blockchain&&) = delete;
    ~PersistentBlockchain() override = default;
    //===================
    void load();
    //===================
    AdditionResult tryAddBlock(const ImmutableBlock& block) override;
    //===================
  private:
    base::Database _database;
    mutable std::shared_mutex _database_rw_mutex;
    //===================
    void pushForwardToPersistentStorage(const ImmutableBlock& block);
    std::optional<base::Sha256> getLastBlockHashAtPersistentStorage() const;
    std::optional<ImmutableBlock> findBlockAtPersistentStorage(const base::Sha256& block_hash) const;
    std::vector<base::Sha256> createAllBlockHashesListAtPersistentStorage() const;
    //===================
};


} // namespace lk
