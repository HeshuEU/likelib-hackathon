#include "miner.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"

#include <limits>


namespace
{

bool checkBlockNonce(const bc::Block& block, const base::Bytes& max_hash_value)
{
    return base::sha256(block.serialize()) < max_hash_value;
}


std::optional<NonceInt> mineBlock(Block& block, const base::Bytes& max_hash_value)
{
    static constexpr NonceInt MAX_NONCE = std::numeric_limits<NonceInt>::max();
    static const base::Bytes MAX_HASH_VALUE{0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    for(NonceInt nonce = 0; nonce < MAX_NONCE; ++nonce) {
        block.setNonce(nonce);
        if(checkBlockNonce(block, MAX_HASH_VALUE)) {
            return nonce;
        }
    }
    block.setNonce(MAX_NONCE);
    if(checkBlockNonce(block, MAX_HASH_VALUE)) {
        return MAX_NONCE;
    }
    else {
        return {};
    }
}
} // namespace


namespace bc
{

Miner::Miner()
{
    for(std::size_t i = 0; i < base::config::BC_MINING_THREADS; ++i) {
        _miners_pool.emplace_back(&Miner::miningWorker, this);
    }
}


void Miner::miningWorker() noexcept
{}


} // namespace bc
