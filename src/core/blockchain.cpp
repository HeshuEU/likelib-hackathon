#include "blockchain.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "core/host.hpp"

#include <optional>

namespace
{

enum class DataType
{
    SYSTEM = 1,
    BLOCK = 2,
    PREVIOUS_BLOCK_HASH = 3
};


base::Bytes toBytes(DataType type, const base::Bytes& key)
{
    base::Bytes data;
    data.append(static_cast<base::Byte>(type));
    data.append(key);
    return data;
}


template<std::size_t S>
base::Bytes toBytes(DataType type, const base::FixedBytes<S>& key)
{
    base::Bytes data;
    data.append(static_cast<base::Byte>(type));
    data.append(key.getData(), S);
    return data;
}

const base::Bytes LAST_BLOCK_HASH_KEY{ toBytes(DataType::SYSTEM, base::Bytes("last_block_hash")) };

} // namespace


namespace lk
{

Blockchain::Blockchain(const base::PropertyTree& config)
  : _config{ config }
  , _top_level_block_hash(base::Bytes(32))
{
    auto database_path = config.get<std::string>("database.path");
    if (config.get<bool>("database.clean")) {
        _database = base::createClearDatabaseInstance(base::Directory(database_path));
        LOG_INFO << "Created clear database instance.";
    }
    else {
        _database = base::createDefaultDatabaseInstance(base::Directory(database_path));
        LOG_INFO << "Loaded database by path: " << database_path;
    }
}


void Blockchain::load()
{
    for (const auto& block_hash : createAllBlockHashesListAtPersistentStorage()) {
        LOG_DEBUG << "Loading block " << block_hash << " from database";
        auto current_block = findBlockAtPersistentStorage(block_hash);
        ASSERT(current_block);
        tryAddBlock(current_block.value());
    }
}


void Blockchain::addGenesisBlock(const Block& block)
{
    auto hash = base::Sha256::compute(base::toBytes(block));

    std::lock_guard lk(_blocks_mutex);
    if (!_blocks.empty()) {
        RAISE_ERROR(base::LogicError, "cannot add genesis to non-empty chain");
    }

    auto inserted_block = _blocks.insert({ hash, block }).first;
    pushForwardToPersistentStorage(hash, block);
    _top_level_block_hash = hash;

    LOG_DEBUG << "Adding genesis block. Block hash = " << hash;
    _block_added.notify(inserted_block->second);
}


bool Blockchain::tryAddBlock(const Block& block)
{
    auto hash = base::Sha256::compute(base::toBytes(block));

    decltype(_blocks)::iterator inserted_block;
    {
        std::lock_guard lk(_blocks_mutex);
        if (!_blocks.empty() && _blocks.find(hash) != _blocks.end()) {
            return false;
        }
        else if (!_blocks.empty() && _top_level_block_hash != block.getPrevBlockHash()) {
            return false;
        }
        else if (_blocks.size() != block.getDepth()) {
            return false;
        }
        else {
            inserted_block = _blocks.insert({ hash, block }).first;
            _blocks_by_depth.insert({ block.getDepth(), hash });
            pushForwardToPersistentStorage(hash, block);
            _top_level_block_hash = hash;
        }
    }

    LOG_DEBUG << "Adding block. Block hash = " << hash;
    _block_added.notify(inserted_block->second);

    return true;
}


std::optional<Block> Blockchain::findBlock(const base::Sha256& block_hash) const
{
    std::shared_lock lk(_blocks_mutex);
    if (auto it = _blocks.find(block_hash); it != _blocks.end()) {
        return it->second;
    }
    else {
        return std::nullopt;
    }
}


std::optional<base::Sha256> Blockchain::findBlockHashByDepth(lk::BlockDepth depth) const
{
    std::shared_lock lk(_blocks_mutex);
    if (auto it = _blocks_by_depth.find(depth); it != _blocks_by_depth.end()) {
        return it->second;
    }
    else {
        return std::nullopt;
    }
}


std::optional<lk::Transaction> Blockchain::findTransaction(const base::Sha256& tx_hash) const
{
    std::shared_lock lk(_blocks_mutex);
    for (const auto& block : _blocks) {
        for (const auto& tx : block.second.getTransactions()) {
            if (base::Sha256::compute(base::toBytes(tx)) == tx_hash) {
                return tx;
            }
        }
    }
    return std::nullopt;
}


const Block& Blockchain::getTopBlock() const
{
    std::shared_lock lk(_blocks_mutex);
    auto it = _blocks.find(_top_level_block_hash);
    ASSERT(it != _blocks.end());
    return it->second;
}


base::Sha256 Blockchain::getTopBlockHash() const
{
    return _top_level_block_hash;
}


void Blockchain::pushForwardToPersistentStorage(const base::Sha256& block_hash, const Block& block)
{
    auto block_data = base::toBytes(block);
    {
        std::lock_guard lk(_database_rw_mutex);
        if (_database.exists(toBytes(DataType::BLOCK, block_hash.getBytes()))) {
            return;
        }
        _database.put(toBytes(DataType::BLOCK, block_hash.getBytes()), block_data);
        _database.put(toBytes(DataType::PREVIOUS_BLOCK_HASH, block_hash.getBytes()),
                      block.getPrevBlockHash().getBytes());
        _database.put(LAST_BLOCK_HASH_KEY, block_hash.getBytes());
    }
}


std::optional<base::Sha256> Blockchain::getLastBlockHashAtPersistentStorage() const
{
    if (_database.exists(LAST_BLOCK_HASH_KEY)) {
        auto hash_data = _database.get(LAST_BLOCK_HASH_KEY);
        if (hash_data) {
            return base::Sha256(std::move(hash_data.value()));
        }
    }
    return std::nullopt;
}


std::optional<Block> Blockchain::findBlockAtPersistentStorage(const base::Sha256& block_hash) const
{
    std::shared_lock lk(_database_rw_mutex);
    auto block_data = _database.get(toBytes(DataType::BLOCK, block_hash.getBytes()));
    if (!block_data) {
        return std::nullopt;
    }
    base::SerializationIArchive ia(block_data.value());
    return lk::Block::deserialize(ia);
}


std::vector<base::Sha256> Blockchain::createAllBlockHashesListAtPersistentStorage() const
{
    std::vector<base::Sha256> all_blocks_hashes{};
    auto last_block_hash = getLastBlockHashAtPersistentStorage();
    if (last_block_hash) {
        base::Sha256 current_block_hash = last_block_hash.value();

        std::shared_lock lk(_database_rw_mutex);
        while (current_block_hash != base::Bytes(32)) {
            all_blocks_hashes.push_back(current_block_hash);
            auto previous_block_hash_data =
              _database.get(toBytes(DataType::PREVIOUS_BLOCK_HASH, current_block_hash.getBytes()));
            ASSERT(previous_block_hash_data);
            current_block_hash = base::Sha256(std::move(previous_block_hash_data.value()));
        }

        std::reverse(std::begin(all_blocks_hashes), std::end(all_blocks_hashes));
    }
    return all_blocks_hashes;
}

} // namespace lk
