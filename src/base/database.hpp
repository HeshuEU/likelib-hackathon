#pragma once

#include "base/bytes.hpp"

#include <leveldb/db.h>

#include <string>
#include <filesystem>
#include <memory>

namespace base
{

enum class BaseDataType
{
    UNDEFINED = 1,
    BLOCK = 2,
    TRANSACTION = 3
};


class DatabaseKey
{
  public:
    DatabaseKey(BaseDataType type, const Bytes& key);

    DatabaseKey(const Bytes& from_bytes);

    Bytes toBytes() const;

    BaseDataType type() const;

    Bytes key() const;

  private:
    BaseDataType _type;
    Bytes _key;
};

class Database
{
  public:
    explicit Database(std::filesystem::path const& path);

    ~Database() = default;

    Bytes get(const DatabaseKey& key) const;

    bool exists(const DatabaseKey& key) const;

    void put(const DatabaseKey& key, const Bytes& value);

    void remove(const DatabaseKey& key);

  private:
    std::unique_ptr<leveldb::DB> _data_base;
    leveldb::ReadOptions const _read_options;
    leveldb::WriteOptions const _write_options;

    static leveldb::ReadOptions defaultReadOptions();

    static leveldb::WriteOptions defaultWriteOptions();

    static leveldb::Options defaultDBOptions();
};


} // namespace base