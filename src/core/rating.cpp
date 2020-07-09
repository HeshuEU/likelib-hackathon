#include "rating.hpp"

#include <algorithm>

namespace lk
{

RatingManager::RatingManager(const base::PropertyTree& config)
  : _db{ config.get<std::string>("net.peers_db") }
{}


Rating RatingManager::get(const net::Endpoint& ep)
{
    return Rating{ ep, _db };
}


void Rating::Data::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(value);
    oa.serialize(ts);
}


Rating::Data Rating::Data::deserialize(base::SerializationIArchive& ia)
{
    auto value = ia.deserialize<Value>();
    auto ts = ia.deserialize<base::Time>();
    return { value, ts };
}


Rating::Rating(const net::Endpoint& ep, base::Database& db)
  : _serialized_ep{ base::toBytes(ep) }
  , _db{ db }
{
    if (auto ret = _db.get(_serialized_ep)) {
        _data = base::fromBytes<Data>(*ret);
    }
    else {
        _data.value = INITIAL_PEER_RATING;
        _data.ts = base::Time{};
        dbUpdate();
    };
}


Rating::Value Rating::getValue()
{
    if (isGood()) {
        return _data.value;
    }
    else {
        const auto current_time = base::Time::now();
        static constexpr unsigned SECONDS_IN_HOUR = 3600;
        Value hours_passed = (current_time - _data.ts).getSeconds() / SECONDS_IN_HOUR;
        static constexpr Rating::Value RATING_VALUE_REPAIRED_EACH_HOUR = 1;
        Value new_value =
          std::min(int(INITIAL_PEER_RATING), _data.value + hours_passed * RATING_VALUE_REPAIRED_EACH_HOUR);
        if (_data.value != new_value) {
            _data.value = new_value;
            _data.ts = current_time;
            dbUpdate();
        }
        return _data.value;
    }
}


void Rating::dbUpdate()
{
    _db.put(_serialized_ep, base::toBytes(_data));
}


bool Rating::isGood() const noexcept
{
    return _data.value > 0;
}


Rating::operator bool() const noexcept
{
    return isGood();
}


Rating& Rating::nonExpectedMessage()
{
    _data.value -= 20;
    dbUpdate();
    return *this;
}


Rating& Rating::invalidMessage()
{
    _data.value -= 30;
    dbUpdate();
    return *this;
}


Rating& Rating::badBlock()
{
    _data.value -= 10;
    dbUpdate();
    return *this;
}


Rating& Rating::differentGenesis()
{
    _data.value -= 2 * INITIAL_PEER_RATING;
    dbUpdate();
    return *this;
}


Rating& Rating::connectionRefused()
{
    _data.value = -INITIAL_PEER_RATING - 10;
    dbUpdate();
    return *this;
}


Rating& Rating::cannotAddToPool()
{
    _data.value -= 10;
    dbUpdate();
    return *this;
}

}