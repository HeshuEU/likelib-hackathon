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

char enumToChar(base::BaseDataType type)
{
    auto temp = underlying_value(type);
    return temp;
}

base::BaseDataType charToEnum(char input)
{
    auto temp = to_enum<base::BaseDataType>(input);
    return temp;
}

} // namespace

namespace base
{

DatabaseKey::DatabaseKey(BaseDataType type, const Bytes& key) : _type(type), _key(key)
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

BaseDataType DatabaseKey::type() const
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

Database::Database(std::filesystem::path const& path)
    : _read_options(defaultReadOptions()), _write_options(defaultWriteOptions())
{
    leveldb::Options db_options = defaultDBOptions();
    leveldb::DB* data_base = nullptr;
    auto const status = leveldb::DB::Open(db_options, path.string(), &data_base);
    if(!status.ok() || data_base == nullptr) {
        RAISE_ERROR(base::DataBaseError, "Failed to create data base instance.");
    }
    _data_base.reset(data_base);
}

Bytes Database::get(const DatabaseKey& key) const
{
    std::string value;
    auto const status = _data_base->Get(_read_options, key.toBytes().toString(), &value);
    if(status.IsNotFound())
        return Bytes();
    if(!status.ok()) {
        RAISE_ERROR(base::DataBaseError, "LevelDBDataBaseError");
    }
    return Bytes(value);
}

bool Database::exists(const DatabaseKey& key) const
{
    std::string value;
    auto const status = _data_base->Get(_read_options, key.toBytes().toString(), &value);
    if(status.IsNotFound())
        return false;
    if(!status.ok()) {
        RAISE_ERROR(base::DataBaseError, "LevelDBDataBaseError");
    }
    return true;
}

void Database::put(const DatabaseKey& key, const Bytes& value)
{
    auto const status = _data_base->Put(_write_options, key.toBytes().toString(), value.toString());
    if(!status.ok()) {
        RAISE_ERROR(base::DataBaseError, "LevelDBDataBaseError");
    }
}

void Database::remove(const DatabaseKey& key)
{
    auto const status = _data_base->Delete(_write_options, key.toBytes().toString());
    if(!status.ok()) {
        RAISE_ERROR(base::DataBaseError, "LevelDBDataBaseError");
    }
}

} // namespace base