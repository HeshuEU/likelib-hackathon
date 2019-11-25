#include "miner.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"

#include <ctime>
#include <cstdlib>
#include <limits>


namespace
{

bool checkBlockNonce(const bc::Block& block, const base::Bytes& max_hash_value)
{
    return base::Sha256::compute(base::toBytes(block)).getBytes() < max_hash_value;
}

} // namespace


namespace bc
{

Miner::Miner()
{}


Miner::~Miner()
{
    stop();
}


void Miner::findNonce(const Block& block)
{
    stop();
    _thread_pool.clear();
    _block_sample = std::move(block);
    _is_stopping = false;
    for(std::size_t i = 0; i < base::config::BC_MINING_THREADS; ++i) {
        _thread_pool.emplace_back(&Miner::miningWorker, this);
    }
}


void Miner::miningWorker() noexcept
{
    std::srand(std::time(nullptr));
    Block block = _block_sample;
    static constexpr bc::NonceInt MAX_NONCE = std::numeric_limits<bc::NonceInt>::max();
    static const base::Bytes MAX_HASH_VALUE = getComplexity();

    unsigned long long iteration_counter = 0;
    for(bc::NonceInt nonce = 0; !_is_stopping && nonce < MAX_NONCE; nonce += std::rand() % 5 + 1) {
        block.setNonce(nonce);
        if(++iteration_counter % 100000 == 0) {
            LOG_DEBUG << "Trying nonce " << nonce << ". Resulting hash "
                      << base::Sha256::compute(base::toBytes(block)).toHex() << ' ' << MAX_HASH_VALUE.toHex();
        }
        if(checkBlockNonce(block, MAX_HASH_VALUE)) {
            if(!_is_stopping) {
                _is_stopping = true;
                _callback(std::move(block));
                return;
            }
        }
    }

    if(!_is_stopping) {
        _callback({});
    }
}


void Miner::setCallback(CallbackType&& callback)
{
    _callback = std::move(callback);
}


void Miner::stop()
{
    _is_stopping = true;
    for(auto& thread: _thread_pool) {
        if(thread.joinable()) {
            thread.join();
        }
    }
}


base::Bytes getComplexity()
{
    base::Bytes ret(32);
    ret[2] = 0x1A;
    return ret;
}


} // namespace bc
