#pragma once

#include "base/bytes.hpp"
#include "base/directory.hpp"

#include <leveldb/db.h>

#include <string>
#include <memory>

namespace base
{

class DatabaseKey
{
  public:
    enum class DataType
    {
        UNDEFINED = 1,
        BLOCK = 2,
        TRANSACTION = 3
    };
    //====================
    DatabaseKey(DataType type, const Bytes& key) noexcept;
    DatabaseKey(const Bytes& from_bytes);
    //====================
    Bytes toBytes() const;
    //====================
    DataType type() const;
    Bytes key() const;
    //====================
  private:
    DataType _type;
    Bytes _key;
    //====================
};

class Database
{
  public:
    Database(Directory const& path);
    ~Database() = default;
    //=====================
    Bytes get(const DatabaseKey& key) const;
    bool exists(const DatabaseKey& key) const;
    void put(const DatabaseKey& key, const Bytes& value);
    void remove(const DatabaseKey& key);
    //=====================
  private:
    //======================
    std::unique_ptr<leveldb::DB> _data_base;
    leveldb::ReadOptions const _read_options;
    leveldb::WriteOptions const _write_options;
    //=====================
    static leveldb::ReadOptions defaultReadOptions();
    static leveldb::WriteOptions defaultWriteOptions();
    static leveldb::Options defaultDBOptions();
    //=====================
};

std::unique_ptr<Database> createDefaultDatabaseInstance(Directory const& path);
std::unique_ptr<Database> createClearDatabaseInstance(Directory const& path);

} // namespace base