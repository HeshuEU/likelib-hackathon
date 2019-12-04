#include "database.hpp"

#include "base/error.hpp"

#include <type_traits>

namespace
{

namespace details
{

    template<typename E>
    using enable_enum_t = typename std::enable_if<std::is_enum<E>::value, typename std::underlying_type<E>::type>::type;

} // namespace details

template<typename E>
constexpr inline details::enable_enum_t<E> underlying_value(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

template<typename E, typename T>
constexpr inline typename std::enable_if<std::is_enum<E>::value && std::is_integral<T>::value, E>::type to_enum(
    T value) noexcept
{
    return static_cast<E>(value);
}

char enumToChar(base::DatabaseKey::DataType type)
{
    auto temp = underlying_value(type);
    return temp;
}

base::DatabaseKey::DataType charToEnum(char input)
{
    auto temp = to_enum<base::DatabaseKey::DataType>(input);
    return temp;
}

} // namespace

namespace base
{

DatabaseKey::DatabaseKey(DataType type, const Bytes& key) noexcept : _type(type), _key(key)
{}


DatabaseKey::DatabaseKey(const Bytes& from_bytes)
{
    _type = charToEnum(from_bytes[0]);
    _key = Bytes(from_bytes.takePart(1, from_bytes.size()));
}


Bytes DatabaseKey::toBytes() const
{
    Bytes type;
    type.append(enumToChar(_type));
    return Bytes(type.toString() + _key.toString()); // TODO: add concatenate method to bytes
}


DatabaseKey::DataType DatabaseKey::type() const
{
    return _type;
}


Bytes DatabaseKey::key() const
{
    return _key;
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


Bytes Database::get(const DatabaseKey& key) const
{
    std::string value;
    auto const status = _data_base->Get(_read_options, key.toBytes().toString(), &value);
    if(status.IsNotFound())
        return Bytes();
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
    return Bytes(value);
}


bool Database::exists(const DatabaseKey& key) const
{
    std::string value;
    auto const status = _data_base->Get(_read_options, key.toBytes().toString(), &value);
    if(status.IsNotFound()) {
        return false;
    }
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
    return true;
}


void Database::put(const DatabaseKey& key, const Bytes& value)
{
    auto const status = _data_base->Put(_write_options, key.toBytes().toString(), value.toString());
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}


void Database::remove(const DatabaseKey& key)
{
    auto const status = _data_base->Delete(_write_options, key.toBytes().toString());
    if(!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}


std::unique_ptr<Database> createDefaultDatabaseInstance(Directory const& path)
{
    return std::make_unique<base::Database>(path);
}


std::unique_ptr<Database> createClearDatabaseInstance(Directory const& path)
{
    if(std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
    return createDefaultDatabaseInstance(path);
}


Database::Database(Directory const& path) : _read_options(defaultReadOptions()), _write_options(defaultWriteOptions())
{
    leveldb::DB* data_base = nullptr;
    auto const status = leveldb::DB::Open(defaultDBOptions(), path.string(), &data_base);
    if(!status.ok() || data_base == nullptr) {
        RAISE_ERROR(base::DatabaseError, "Failed to create data base instance.");
    }
    _data_base.reset(data_base);
}

} // namespace base