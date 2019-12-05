#pragma once

#include "base/bytes.hpp"
#include "base/directory.hpp"

#include <leveldb/db.h>

#include <memory>

namespace base
{

class Database
{
  public:
    explicit Database(Directory const& path);
    ~Database() = default;
    //=====================
    void get(const Bytes& key, Bytes& res) const;
    bool exists(const Bytes& key) const;
    void put(const Bytes& key, const Bytes& value);
    void remove(const Bytes& key);
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