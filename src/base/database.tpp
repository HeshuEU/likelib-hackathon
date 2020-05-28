#include "database.hpp"

namespace base
{
template<typename B1, typename B2>
void Database::put(const B1& key, const B2& value)
{
    checkStatus();

    auto const status = _database->Put(_write_options, key.toString(), value.toString());
    if (!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}


template<typename B>
std::optional<Bytes> Database::get(const B& key) const
{
    checkStatus();

    std::string value;
    auto const status = _database->Get(_read_options, key.toString(), &value);
    if (!status.ok()) {
        return std::nullopt;
    }
    return Bytes(value);
}


template<typename B>
bool Database::exists(const B& key) const
{
    checkStatus();

    std::string value;
    auto const status = _database->Get(_read_options, key.toString(), &value);
    if (status.IsNotFound()) {
        return false;
    }
    if (!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }

    return true;
}


template<typename B>
void Database::remove(const B& key)
{
    checkStatus();

    auto const status = _database->Delete(_write_options, key.toString());
    if (!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}

} // namespace base
