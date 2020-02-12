#include "database.hpp"

#include "base/error.hpp"
#include "base/config.hpp"

#include <leveldb/cache.h>

namespace base
{

Database::Database(Directory const& path)
{
    open(path);
}


void Database::open(Directory const& path)
{
    if(_inited) {
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
    if constexpr(config::DATABASE_COMPRESS_DATA) {
        database_options.compression = leveldb::kSnappyCompression; // fast compression
    }
    else {
        database_options.compression = leveldb::kNoCompression; // no compress data
    }
    database_options.block_cache = leveldb::NewLRUCache(config::DATABASE_DATA_BLOCK_CACHE_SIZE);

    // create database
    leveldb::DB* data_base = nullptr;
    auto const status = leveldb::DB::Open(database_options, path.string(), &data_base);
    if(!status.ok() || data_base == nullptr) {
        RAISE_ERROR(base::DatabaseError, "Failed to create database instance.");
    }
    _database.reset(data_base);

    // set up guard flag
    _inited = true;
}


std::optional<Bytes> Database::get(const Bytes& key) const
{
    checkStatus();

    std::string value;
    auto const status = _database->Get(_read_options, key.toString(), &value);
    if(!status.ok()) {
        return std::nullopt;
    }
    return Bytes(value);
}


bool Database::exists(const Bytes& key) const
{
    checkStatus();

    std::string value;
    auto const status = _database->Get(_read_options, key.toString(), &value);
    if(status.IsNotFound()) {
        return false;
    }
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }

    return true;
}


void Database::put(const Bytes& key, const Bytes& value)
{
    checkStatus();

    auto const status = _database->Put(_write_options, key.toString(), value.toString());
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}


void Database::remove(const Bytes& key)
{
    checkStatus();

    auto const status = _database->Delete(_write_options, key.toString());
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}


void Database::checkStatus() const
{
    if(!_inited) {
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
    if(std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
    return createDefaultDatabaseInstance(path);
}


} // namespace base