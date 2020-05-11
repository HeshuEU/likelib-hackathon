#include "peer.hpp"

#include "base/serialization.hpp"

namespace lk::msg
{

void Connect::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(address);
    oa.serialize(public_port);
    oa.serialize(top_block_hash);
}


Connect Connect::deserialize(base::SerializationIArchive& ia)
{
    auto address = ia.deserialize<lk::Address>();
    auto peers_id = ia.deserialize<std::uint16_t>();
    auto top_block_hash = ia.deserialize<base::Sha256>();
    return Connect{ std::move(address), peers_id, std::move(top_block_hash) };
}


void CannotAccept::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(why_not_accepted);
    oa.serialize(peers_info);
}


CannotAccept CannotAccept::deserialize(base::SerializationIArchive& ia)
{
    auto why_not_accepted = ia.deserialize<CannotAccept::RefusionReason>();
    auto peers_info = ia.deserialize<std::vector<Peer::IdentityInfo>>();
    return CannotAccept{ std::move(why_not_accepted), std::move(peers_info) };
}


void Accepted::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(address);
    oa.serialize(public_port);
    oa.serialize(top_block_hash);
}


Accepted Accepted::deserialize(base::SerializationIArchive& ia)
{
    auto address = ia.deserialize<lk::Address>();
    auto public_port = ia.deserialize<std::uint16_t>();
    auto top_block_hash = ia.deserialize<base::Sha256>();
    return Accepted{ std::move(address), public_port, std::move(top_block_hash) };
}


void Ping::serialize(base::SerializationOArchive&) const {}


Ping Ping::deserialize(base::SerializationIArchive&)
{
    return Ping{};
}


void Pong::serialize(base::SerializationOArchive&) const {}


Pong Pong::deserialize(base::SerializationIArchive&)
{
    return Pong{};
}


void Lookup::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(address);
    oa.serialize(selection_size);
}


Lookup Lookup::deserialize(base::SerializationIArchive& ia)
{
    auto address = ia.deserialize<lk::Address>();
    auto selection_size = ia.deserialize<std::uint8_t>();
    return Lookup{ std::move(address), selection_size };
}


void LookupResponse::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(address);
    oa.serialize(peers_info);
}


LookupResponse LookupResponse::deserialize(base::SerializationIArchive& ia)
{
    auto address = ia.deserialize<lk::Address>();
    auto peers_info = ia.deserialize<std::vector<Peer::IdentityInfo>>();
    return LookupResponse{ std::move(address), std::move(peers_info) };
}


void Transaction::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(tx);
}


Transaction Transaction::deserialize(base::SerializationIArchive& ia)
{
    auto tx = lk::Transaction::deserialize(ia);
    return Transaction{ std::move(tx) };
}


void GetBlock::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(block_hash);
}


GetBlock GetBlock::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return GetBlock{ std::move(block_hash) };
}


void Block::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(block);
}


Block Block::deserialize(base::SerializationIArchive& ia)
{
    auto block = lk::Block::deserialize(ia);
    return Block{ std::move(block) };
}


void BlockNotFound::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(block_hash);
}


BlockNotFound BlockNotFound::deserialize(base::SerializationIArchive& ia)
{
    auto block_hash = base::Sha256::deserialize(ia);
    return BlockNotFound{ std::move(block_hash) };
}


void Close::serialize(base::SerializationOArchive&) const {}


Close Close::deserialize(base::SerializationIArchive&)
{
    return Close{};
}

}