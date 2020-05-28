#pragma once

#include "base/bytes.hpp"
#include "base/directory.hpp"

#include <leveldb/cache.h>
#include <leveldb/db.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

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
    template<typename B> // expects base::Bytes or base::FixedBytes<>
    [[nodiscard]] std::optional<Bytes> get(const B& key) const;

    template<typename B>
    bool exists(const B& key) const;

    template<typename B1, typename B2>
    void put(const B1& key, const B2& value);

    template<typename B>
    void remove(const B& key);
    //======================
  private:
    //======================
    bool _inited{ false };
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

#include "database.tpp"
