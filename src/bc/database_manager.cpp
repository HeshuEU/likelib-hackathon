#include "database_manager.hpp"

#include <algorithm>

namespace
{

enum class DataType
{
    UNDEFINED = 1,
    BLOCK = 2,
    TRANSACTION = 3
};

base::Bytes toBytes(DataType type, const base::Bytes& key)
{
    base::Bytes data;
    data.append(static_cast<base::Byte>(type));
    data.append(key);
    return data;
}

} // namespace


namespace bc
{

const base::Bytes LAST_BLOCK_HASH_KEY(toBytes(DataType::UNDEFINED, base::Bytes("last_block_hash")));


DatabaseManager::DatabaseManager(const base::PropertyTree& config) : _last_block_hash(base::Bytes(32))
{
    auto database_path = config.get<std::string>("database.path");
    if(config.get<bool>("database.clean")) {
        _database = base::createClearDatabaseInstance(base::Directory(database_path));
    }
    else {
        _database = base::createDefaultDatabaseInstance(base::Directory(database_path));
        if(!_database.exists(LAST_BLOCK_HASH_KEY)) {
            RAISE_ERROR(base::InvalidArgument, "Database has no block data");
        }
        else {
            base::Bytes hash_data;
            _database.get(LAST_BLOCK_HASH_KEY, hash_data);
            _last_block_hash = base::Sha256(std::move(hash_data));
        }
    }
}


base::Sha256 DatabaseManager::addBlock(const bc::Block& block)
{
    std::lock_guard lk(_rw_mutex);
    auto block_data = base::toBytes(block);
    auto block_hash = base::Sha256::compute(block_data);
    _database.put(toBytes(DataType::BLOCK, block_hash.getBytes()), block_data);
    _database.put(LAST_BLOCK_HASH_KEY, block_hash.getBytes());
    _last_block_hash = block_hash;
    return block_hash;
}


bool DatabaseManager::isBlockExists(const base::Sha256& blockHash) const
{
    std::shared_lock lk(_rw_mutex);
    return _database.exists(toBytes(DataType::BLOCK, blockHash.getBytes()));
}


bc::Block DatabaseManager::getBlock(const base::Sha256& blockHash) const
{
    if(isBlockExists(blockHash)) {
        std::shared_lock lk(_rw_mutex);
        base::Bytes block_data;
        _database.get(toBytes(DataType::BLOCK, blockHash.getBytes()), block_data);
        return base::fromBytes<::bc::Block>(block_data);
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "No such block exists");
    }
}


const base::Sha256& DatabaseManager::getLastBlockHash() const noexcept
{
    return _last_block_hash;
}


std::vector<base::Sha256> DatabaseManager::createAllBlockHashesList()
{
    // TODO: replace for blocks iterator
    std::shared_lock lk(_rw_mutex);
    std::vector<::base::Sha256> all_blocks_hashes{};
    base::Sha256 current_block_hash = getLastBlockHash();
    while(current_block_hash != base::Bytes(32)) {
        all_blocks_hashes.push_back(current_block_hash);
        base::Bytes block_data;
        _database.get(toBytes(DataType::BLOCK, current_block_hash.getBytes()), block_data);
        current_block_hash = base::fromBytes<::bc::Block>(block_data).getPrevBlockHash();
    }
    std::reverse(std::begin(all_blocks_hashes), std::end(all_blocks_hashes));
    return all_blocks_hashes;
}

} // namespace bc
