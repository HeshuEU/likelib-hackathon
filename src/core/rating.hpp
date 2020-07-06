#pragma once

#include "base/bytes.hpp"
#include "base/config.hpp"
#include "base/database.hpp"
#include "base/property_tree.hpp"
#include "base/serialization.hpp"
#include "base/time.hpp"
#include "net/endpoint.hpp"

#include <cstdint>
#include <type_traits>

namespace lk
{

class Rating
{
  public:
    using Value = std::int16_t;

    Rating(const net::Endpoint& ep, base::Database& db);

    Value getValue();

    explicit operator bool() const noexcept; // if false, then the peer is too bad
    bool isGood() const noexcept;

    Rating& nonExpectedMessage();
    Rating& invalidMessage();
    Rating& badBlock();
    Rating& differentGenesis();
    Rating& connectionRefused();
    Rating& cannotAddToPool();

  private:
    static constexpr Value INITIAL_PEER_RATING = 20;

    const base::Bytes _serialized_ep;
    base::Database& _db;

    struct Data
    {
        Value value;
        base::Time ts;

        void serialize(base::SerializationOArchive& oa) const;
        static Data deserialize(base::SerializationIArchive& ia);
    };
    Data _data;

    void dbUpdate();
};


class RatingManager
{
  public:
    RatingManager(const base::PropertyTree& config);

    Rating get(const net::Endpoint& ep);

  private:
    base::Database _db;
};

}