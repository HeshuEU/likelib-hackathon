#pragma once

#include "bc/block.hpp"
#include "bc/balance_manager.hpp"

#include "base/database.hpp"
#include "base/property_tree.hpp"

#include <list>
#include <shared_mutex>

namespace bc
{

class DatabaseManager
{
  public:
    explicit DatabaseManager(const ::base::PropertyTree& config);
    ~DatabaseManager() = default;
    //=====================
    ::base::Sha256 addBlock(const ::bc::Block& block);
    bool isBlockExists(const ::base::Sha256& blockHash) const;
    ::bc::Block getBlock(const ::base::Sha256& blockHash) const;
    //=====================
    const ::base::Sha256& getLastBlockHash() const noexcept;
    //=====================
    ::std::list<::base::Sha256> createAllBlockHashesList();
    //=====================
  private:
    //=====================
    base::Database _database;
    mutable std::shared_mutex _rw_mutex;
    ::base::Sha256 _last_block_hash;
    //==============================
};

} // namespace bc