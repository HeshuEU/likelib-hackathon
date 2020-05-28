#include "database.hpp"

#include "base/config.hpp"
#include "base/error.hpp"

#include <leveldb/cache.h>

namespace base
{

Database::Database(Directory const& path)
{
    open(path);
}


void Database::open(Directory const& path)
{
    if (_inited) {
        RAISE_ERROR(LogicError, "Double initialization of one database instance");
    }
    // reset and set up read options
    _read_options = leveldb::ReadOptions{};
    _read_options.fill_cache = true;

    // reset and set up write options
    _write_options = leveldb::WriteOptions{};
    _write_options.sync = true;

    // set up database init options;
    leveldb::Options database_options;
    database_options.create_if_missing = true;
    database_options.write_buffer_size = config::DATABASE_WRITE_BUFFER_SIZE;
    database_options.block_size = config::DATABASE_DATA_BLOCK_SIZE;
    if constexpr (config::DATABASE_COMPRESS_DATA) {
        database_options.compression = leveldb::kSnappyCompression; // fast compression
    }
    else {
        database_options.compression = leveldb::kNoCompression; // no compress data
    }

    _cache = std::unique_ptr<leveldb::Cache>(leveldb::NewLRUCache(config::DATABASE_DATA_BLOCK_CACHE_SIZE));
    database_options.block_cache = _cache.get();

    // create database
    leveldb::DB* database = nullptr;
    auto const status = leveldb::DB::Open(database_options, path.string(), &database);
    if (!status.ok() || database == nullptr) {
        RAISE_ERROR(base::DatabaseError, "Failed to create database instance.");
    }
    _database.reset(database);

    // set up guard flag
    _inited = true;
}


void Database::checkStatus() const
{
    if (!_inited) {
        RAISE_ERROR(base::DatabaseError, "Database is not inited yet");
    }
}


Database createDefaultDatabaseInstance(Directory const& path)
{
    createIfNotExists(path);
    return Database(path);
}


Database createClearDatabaseInstance(Directory const& path)
{
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
    return createDefaultDatabaseInstance(path);
}


} // namespace base