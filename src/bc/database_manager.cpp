#include "database_manager.hpp"

namespace bc
{

const base::DatabaseKey LAST_BLOCK_HASH_KEY{base::DatabaseKey::DataType::UNDEFINED, base::Bytes("last_block_hash")};


DatabaseManager::DatabaseManager(const base::PropertyTree& config) : _last_block_hash(base::getNullSha256())
{
    auto database_path = config.get<std::string>("database.path");
    if(config.get<bool>("database.clean")) {
        _database = base::createClearDatabaseInstance(base::Directory(database_path));
    }
    else {
        _database = base::createDefaultDatabaseInstance(base::Directory(database_path));
        if(!_database->exists(LAST_BLOCK_HASH_KEY)) {
            RAISE_ERROR(::base::InvalidArgument, "Database has no block data");
        }
        else {
            _last_block_hash = ::base::Sha256(_database->get(LAST_BLOCK_HASH_KEY));
        }
    }
}


base::Sha256 DatabaseManager::addBlock(const bc::Block& block)
{
    std::shared_lock lk(_rw_mutex);
    auto block_data = base::toBytes(block);
    auto block_hash = base::Sha256::compute(block_data);
    _database->put(base::DatabaseKey{base::DatabaseKey::DataType::BLOCK, block_hash.getBytes()}, block_data);
    _database->put(LAST_BLOCK_HASH_KEY, block_hash.getBytes());
    _last_block_hash = block_hash;
    return block_hash;
}


bool DatabaseManager::isBlockExists(const base::Sha256& blockHash) const
{
    std::shared_lock lk(_rw_mutex);
    return _database->exists(base::DatabaseKey{base::DatabaseKey::DataType::BLOCK, blockHash.getBytes()});
}


bc::Block DatabaseManager::getBlock(const base::Sha256& blockHash) const
{
    if(isBlockExists(blockHash)) {
        std::shared_lock lk(_rw_mutex);
        auto block_data = _database->get(base::DatabaseKey{base::DatabaseKey::DataType::BLOCK, blockHash.getBytes()});
        return base::fromBytes<::bc::Block>(block_data);
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "No such block exists");
    }
}


base::Sha256 DatabaseManager::getLastBlockHash() const noexcept
{
    return _last_block_hash;
}


std::list<base::Sha256> DatabaseManager::createAllBlockHashesList()
{
    // TODO: replace for blocks iterator
    std::shared_lock lk(_rw_mutex);
    ::std::list<::base::Sha256> all_blocks_hashes{};
    ::base::Sha256 current_block_hash = getLastBlockHash();
    while(current_block_hash != base::getNullSha256()) {
        all_blocks_hashes.push_front(current_block_hash);
        auto block_data =
            _database->get(base::DatabaseKey{base::DatabaseKey::DataType::BLOCK, current_block_hash.getBytes()});
        current_block_hash = base::fromBytes<::bc::Block>(block_data).getPrevBlockHash();
    }
    return all_blocks_hashes;
}

} // namespace bc
