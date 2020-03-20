#include "database.hpp"


namespace base
{
template<std::size_t S>
void Database::put(const Bytes& key, const FixedBytes<S>& value)
{
    checkStatus();

    auto const status = _database->Put(_write_options, key.toString(), value.toString());
    if (!status.ok()) {
        RAISE_ERROR(base::DatabaseError, status.ToString());
    }
}
} // namespace base