#pragma once

#include "base/utility.hpp"
#include "core/address.hpp"
#include "core/block.hpp"
#include "core/transaction.hpp"
#include "net/endpoint.hpp"

namespace lk::msg
{

// clang-format off
DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(Type, std::uint8_t,
  (DEBUG_MIN)
  (CONNECT)
  (CANNOT_ACCEPT)
  (ACCEPTED)
  (PING)
  (PONG)
  (LOOKUP)
  (LOOKUP_RESPONSE)
  (TRANSACTION)
  (GET_BLOCK)
  (BLOCK)
  (BLOCK_NOT_FOUND)
  (NEW_BLOCK)
  (CLOSE)
  (DEBUG_MAX)
)
// clang-format on


struct NodeIdentityInfo
{
    net::Endpoint endpoint;
    lk::Address address;

    static NodeIdentityInfo deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
};


struct Connect
{
    static constexpr Type TYPE_ID = Type::CONNECT;

    lk::Address address;
    std::uint16_t public_port;
    base::Sha256 top_block_hash;

    void serialize(base::SerializationOArchive& oa) const;
    static Connect deserialize(base::SerializationIArchive& ia);
};


struct CannotAccept
{
    static constexpr Type TYPE_ID = Type::CANNOT_ACCEPT;

    DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(RefusionReason,
                                              std::uint8_t,
                                              (NOT_AVAILABLE)(BUCKET_IS_FULL)(BAD_RATING));

    RefusionReason why_not_accepted;
    std::vector<NodeIdentityInfo> peers_info;

    void serialize(base::SerializationOArchive& oa) const;
    static CannotAccept deserialize(base::SerializationIArchive& ia);
};


struct Accepted
{
    static constexpr Type TYPE_ID = Type::ACCEPTED;

    lk::Address address;
    uint16_t public_port; // zero public port states that peer didn't provide information about his public endpoint
    base::Sha256 top_block_hash;

    void serialize(base::SerializationOArchive& oa) const;
    static Accepted deserialize(base::SerializationIArchive& ia);
};


struct Ping
{
    static constexpr Type TYPE_ID = Type::PING;

    void serialize(base::SerializationOArchive& oa) const;
    static Ping deserialize(base::SerializationIArchive& ia);
};


struct Pong
{
    static constexpr Type TYPE_ID = Type::PONG;

    void serialize(base::SerializationOArchive& oa) const;
    static Pong deserialize(base::SerializationIArchive& ia);
};


struct Lookup
{
    static constexpr Type TYPE_ID = Type::LOOKUP;

    lk::Address address;
    std::uint8_t selection_size;

    void serialize(base::SerializationOArchive& oa) const;
    static Lookup deserialize(base::SerializationIArchive& ia);
};


struct LookupResponse
{
    static constexpr Type TYPE_ID = Type::LOOKUP_RESPONSE;

    lk::Address address;
    std::vector<NodeIdentityInfo> peers_info;

    void serialize(base::SerializationOArchive& oa) const;
    static LookupResponse deserialize(base::SerializationIArchive& ia);
};


struct Transaction
{
    static constexpr Type TYPE_ID = Type::TRANSACTION;

    lk::Transaction tx;

    void serialize(base::SerializationOArchive& oa) const;
    static Transaction deserialize(base::SerializationIArchive& ia);
};


struct GetBlock
{
    static constexpr Type TYPE_ID = Type::GET_BLOCK;

    base::Sha256 block_hash;

    void serialize(base::SerializationOArchive& oa) const;
    static GetBlock deserialize(base::SerializationIArchive& ia);
};


struct Block
{
    static constexpr Type TYPE_ID = Type::BLOCK;

    base::Sha256 block_hash;
    ImmutableBlock block;

    void serialize(base::SerializationOArchive& oa) const;
    static Block deserialize(base::SerializationIArchive& ia);
};


struct BlockNotFound
{
    static constexpr Type TYPE_ID = Type::BLOCK_NOT_FOUND;

    base::Sha256 block_hash;

    void serialize(base::SerializationOArchive& oa) const;
    static BlockNotFound deserialize(base::SerializationIArchive& ia);
};


struct NewBlock
{
    static constexpr Type TYPE_ID = Type::NEW_BLOCK;

    base::Sha256 block_hash;
    ImmutableBlock block;

    void serialize(base::SerializationOArchive& oa) const;
    static NewBlock deserialize(base::SerializationIArchive& ia);
};


struct Close
{
    static constexpr Type TYPE_ID = Type::CLOSE;

    void serialize(base::SerializationOArchive& oa) const;
    static Close deserialize(base::SerializationIArchive& ia);
};

}