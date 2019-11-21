#include "miner.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"

#include <limits>
#include <random>


namespace
{

bool checkBlockNonce(const bc::Block& block, const base::Bytes& max_hash_value)
{
    base::SerializationOArchive oa;
    oa << block;
    return base::Sha256::calcSha256(oa.getBytes()).getBytes() < max_hash_value;
}

} // namespace


namespace bc
{

Miner::Miner()
{}


void Miner::findNonce(const Block& block, Miner::CallbackType&& callback)
{
    _is_stopping = true;
    for(auto& t: _miners_pool) {
        if(t.joinable()) {
            t.join();
        }
    }
    _miners_pool.clear();
    _block_sample = std::move(block);
    _callback = std::move(callback);
    _is_stopping = false;
    for(std::size_t i = 0; i < base::config::BC_MINING_THREADS; ++i) {
        _miners_pool.emplace_back(&Miner::miningWorker, this);
    }
}


void Miner::miningWorker() noexcept
{
    Block block = _block_sample;
    static constexpr bc::NonceInt MAX_NONCE = std::numeric_limits<bc::NonceInt>::max();
    static const base::Bytes MAX_HASH_VALUE{0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    for(bc::NonceInt nonce = 0; !_is_stopping && nonce < MAX_NONCE; ++nonce) {
        block.setNonce(nonce);
        if(checkBlockNonce(block, MAX_HASH_VALUE)) {
            _is_stopping = true;
            if(!_is_stopping) {
                _callback(std::move(block));
            }
        }
    }

    if(!_is_stopping) {
        _callback({});
    }
}


} // namespace bc
