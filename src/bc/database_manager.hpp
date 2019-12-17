#pragma once

#include "bc/block.hpp"

#include "base/database.hpp"
#include "base/property_tree.hpp"

#include <shared_mutex>

namespace bc
{

class DatabaseManager
{
  public:
    explicit DatabaseManager(const base::PropertyTree& config);
    ~DatabaseManager() = default;
    //==============================
    void addBlock(const base::Sha256& block_hash, const bc::Block& block);
    bool isBlockExists(const base::Sha256& blockHash) const;
    bc::Block getBlock(const base::Sha256& blockHash) const;
    //==============================
    const base::Sha256& getLastBlockHash() const noexcept;
    //==============================
    std::vector<base::Sha256> createAllBlockHashesList() const;
    //==============================
  private:
    //==============================
    base::Database _database;
    mutable std::shared_mutex _rw_mutex;
    //==============================
    base::Sha256 _last_block_hash;
    //==============================
};

} // namespace bc