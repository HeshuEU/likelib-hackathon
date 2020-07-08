#include "blockchain.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include "core/consensus.hpp"

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

Blockchain::Blockchain(ImmutableBlock genesis_block, const base::PropertyTree& config)
  : _config{ config }
  , _top_level_block_hash{ base::Sha256::null() }
  // temporary null, because it requires initialization. Set to real value in addGenesisBlock
  , _genesis_block_hash{ base::Sha256::null() }
{
    addGenesisBlock(genesis_block);
}


void Blockchain::addGenesisBlock(ImmutableBlock block)
{
    const auto hash = block.getHash();

    std::lock_guard lk(_blocks_mutex);
    if (!_blocks.empty()) {
        RAISE_ERROR(base::LogicError, "cannot add genesis to non-empty chain");
    }

    auto inserted_block = _blocks.insert({ hash, std::move(block) }).first;
    _blocks_by_depth.insert({0, hash});
    _top_level_block_hash = _genesis_block_hash = hash;

    LOG_DEBUG << "Adding genesis block. Block hash = " << hash;
    _block_added.notify(inserted_block->second);
}


Blockchain::AdditionResult Blockchain::tryAddBlock(const ImmutableBlock& block)
{
    const auto hash = block.getHash();

    decltype(_blocks)::iterator inserted_block;
    {
        std::lock_guard lk(_blocks_mutex);

        if (!_blocks.empty() && _blocks.find(hash) != _blocks.end()) {
            return AdditionResult::ALREADY_IN_BLOCKCHAIN;
        }
        else if (!_blocks.empty() && _top_level_block_hash != block.getPrevBlockHash()) {
            return AdditionResult::INVALID_PARENT_HASH;
        }
        else if (_blocks.size() != block.getDepth()) {
            return AdditionResult::INVALID_DEPTH;
        }
        else if (!checkConsensus(block)) {
            return AdditionResult::CONSENSUS_ERROR;
        }
        else if (_top_level_block_hash != block.getPrevBlockHash()) {
            return AdditionResult::INVALID_PARENT_HASH;
        }
        else if (block.getTransactions().size() == 0 ||
                 block.getTransactions().size() > base::config::BC_MAX_TRANSACTIONS_IN_BLOCK) {
            return AdditionResult::INVALID_TRANSACTIONS_NUMBER;
        }
        else if (_getTopBlock().getTimestamp() >= block.getTimestamp()) {
            return AdditionResult::OLD_TIMESTAMP;
        }
        else if (constexpr unsigned SECONDS_IN_DAY = 24 * 60 * 60;
                 block.getTimestamp().getSeconds() > base::Time::now().getSeconds() + SECONDS_IN_DAY) {
            return AdditionResult::FUTURE_TIMESTAMP;
        }

        // if here, this means that the block is ok
        LOG_DEBUG << "Complexity right now is: " << _consensus.getComplexity().getDensed();
        _consensus.applyBlock(block);

        inserted_block = _blocks.insert({ hash, block }).first;
        _blocks_by_depth.insert({ block.getDepth(), hash });
        _top_level_block_hash = hash;
    }

    LOG_DEBUG << "Block " << hash << " has been added to blockchain";
    _block_added.notify(inserted_block->second);

    return AdditionResult::ADDED;
}


std::optional<ImmutableBlock> Blockchain::findBlock(const base::Sha256& block_hash) const
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
            if (tx.hashOfTransaction() == tx_hash) {
                return tx;
            }
        }
    }
    return std::nullopt;
}


ImmutableBlock Blockchain::getGenesisBlock() const
{
    auto block = findBlock(_genesis_block_hash);
    ASSERT(block);
    return *block;
}


std::pair<ImmutableBlock, lk::Complexity> Blockchain::getTopBlockAndComplexity() const
{
    std::shared_lock lk(_blocks_mutex);
    return { getTopBlock(), _consensus.getComplexity() };
}


ImmutableBlock Blockchain::getTopBlock() const
{
    std::shared_lock lk(_blocks_mutex);
    return _getTopBlock();
}


const ImmutableBlock& Blockchain::_getTopBlock() const
{
    ASSERT(!_blocks_mutex.try_lock()); // ensures that this function is used only in thread-safe environment
    auto it = _blocks.find(_top_level_block_hash);
    ASSERT(it != _blocks.end());
    return it->second;
}


base::Sha256 Blockchain::getTopBlockHash() const
{
    return _top_level_block_hash;
}


bool Blockchain::checkConsensus(const ImmutableBlock& block) const
{
    return _consensus.checkBlock(block);
}


PersistentBlockchain::PersistentBlockchain(ImmutableBlock genesis_block, const base::PropertyTree& config)
  : Blockchain{ std::move(genesis_block), config }
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


void PersistentBlockchain::load()
{
    for (const auto& block_hash : createAllBlockHashesListAtPersistentStorage()) {
        LOG_DEBUG << "Loading block " << block_hash << " from database";
        auto current_block = findBlockAtPersistentStorage(block_hash);
        ASSERT(current_block);
        tryAddBlock(current_block.value());
    }
}


IBlockchain::AdditionResult PersistentBlockchain::tryAddBlock(const ImmutableBlock& block)
{
    auto r = Blockchain::tryAddBlock(block);
    if (r == IBlockchain::AdditionResult::ADDED) {
        pushForwardToPersistentStorage(block);
    }
    else {
        LOG_DEBUG << block.getHash() << " is not added with reason " << static_cast<int>(r);
    }
    return r;
}


void PersistentBlockchain::pushForwardToPersistentStorage(const ImmutableBlock& block)
{
    const auto raw_block_hash = block.getHash().getBytes();
    auto serialized_block = base::toBytes(block);
    {
        std::lock_guard lk(_database_rw_mutex);
        if (_database.exists(toBytes(DataType::BLOCK, raw_block_hash))) {
            return;
        }
        _database.put(toBytes(DataType::BLOCK, raw_block_hash), serialized_block);
        _database.put(toBytes(DataType::PREVIOUS_BLOCK_HASH, raw_block_hash), block.getPrevBlockHash().getBytes());
        _database.put(LAST_BLOCK_HASH_KEY, raw_block_hash);
    }
}


std::optional<base::Sha256> PersistentBlockchain::getLastBlockHashAtPersistentStorage() const
{
    if (_database.exists(LAST_BLOCK_HASH_KEY)) {
        auto hash_data = _database.get(LAST_BLOCK_HASH_KEY);
        if (hash_data) {
            return base::Sha256(std::move(hash_data.value()));
        }
    }
    return std::nullopt;
}


std::optional<ImmutableBlock> PersistentBlockchain::findBlockAtPersistentStorage(const base::Sha256& block_hash) const
{
    std::shared_lock lk(_database_rw_mutex);
    auto block_data = _database.get(toBytes(DataType::BLOCK, block_hash.getBytes()));
    if (!block_data) {
        return std::nullopt;
    }
    base::SerializationIArchive ia(block_data.value());
    return ia.deserialize<ImmutableBlock>();
}


std::vector<base::Sha256> PersistentBlockchain::createAllBlockHashesListAtPersistentStorage() const
{
    std::vector<base::Sha256> all_blocks_hashes{};
    auto last_block_hash = getLastBlockHashAtPersistentStorage();
    if (last_block_hash) {
        base::Sha256 current_block_hash = last_block_hash.value();
        const base::Sha256 genesis_hash = getGenesisBlock().getHash();

        std::shared_lock lk(_database_rw_mutex);
        while (current_block_hash != genesis_hash) {
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
