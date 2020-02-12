#pragma once

#include "base/bytes.hpp"
#include "base/directory.hpp"

#include <leveldb/db.h>

#include <string>
#include <filesystem>
#include <memory>
#include <optional>

namespace base
{

class Database
{
  public:
    explicit Database() = default;
    explicit Database(Directory const& path);
    Database(Database&&) = default;
    Database& operator=(Database&&) = default;
    ~Database() = default;
    //======================
    void open(Directory const& path);
    //======================
    [[nodiscard]] std::optional<Bytes> get(const Bytes& key) const;
    [[nodiscard]] bool exists(const Bytes& key) const;
    void put(const Bytes& key, const Bytes& value);
    void remove(const Bytes& key);
    //======================
  private:
    //======================
    bool _inited{false};
    std::unique_ptr<leveldb::DB> _database;
    leveldb::ReadOptions _read_options;
    leveldb::WriteOptions _write_options;

    std::unique_ptr<leveldb::Cache> _cache;
    //=====================
    void checkStatus() const;
    //=====================
};

Database createDefaultDatabaseInstance(Directory const& path);

Database createClearDatabaseInstance(Directory const& path);

} // namespace base
