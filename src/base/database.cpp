#include "database.hpp"

#include "base/error.hpp"

namespace base
{

Database::Database(Directory const& path)
{
    open(path);
}


void Database::open(Directory const& path)
{
    _read_options = defaultReadOptions();
    _write_options = defaultWriteOptions();
    leveldb::DB* data_base = nullptr;
    auto const status = leveldb::DB::Open(defaultDBOptions(), path.string(), &data_base);
    if(!status.ok() || data_base == nullptr) {
        RAISE_ERROR(base::DatabaseError, "Failed to create data base instance.");
    }
    _database.reset(data_base);
    _inited = true;
}


void Database::get(const Bytes& key, Bytes& res) const
{
    if(!_inited) {
        RAISE_ERROR(base::DatabaseError, "Database is not inited yet");
    }
    std::string value;
    auto const status = _database->Get(_read_options, key.toString(), &value);
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
    else {
        res = Bytes(value);
    }
}


bool Database::exists(const Bytes& key) const
{
    if(!_inited) {
        RAISE_ERROR(base::DatabaseError, "Database is not inited yet");
    }
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
    if(!_inited) {
        RAISE_ERROR(base::DatabaseError, "Database is not inited yet");
    }
    auto const status = _database->Put(_write_options, key.toString(), value.toString());
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}


void Database::remove(const Bytes& key)
{
    if(!_inited) {
        RAISE_ERROR(base::DatabaseError, "Database is not inited yet");
    }
    auto const status = _database->Delete(_write_options, key.toString());
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}


leveldb::ReadOptions Database::defaultReadOptions()
{
    return leveldb::ReadOptions();
}


leveldb::WriteOptions Database::defaultWriteOptions()
{
    leveldb::WriteOptions options;
    options.sync = true;
    return options;
}


leveldb::Options Database::defaultDBOptions()
{
    leveldb::Options options;
    options.create_if_missing = true;
    return options;
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