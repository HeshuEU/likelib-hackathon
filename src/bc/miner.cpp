#include "miner.hpp"

#include "base/assert.hpp"
#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"

#include <limits>
#include <random>

namespace bc
{

Miner::Miner()
{
    for(std::size_t i = 0; i < base::config::BC_MINING_THREADS; ++i) {
        _thread_pool.emplace_back(&Miner::miningWorker, this);
    }
}


Miner::~Miner()
{
    stop();
}


void Miner::findNonce(const Block& block, const base::Bytes& complexity)
{
    // stop(); // TODO: need to wait for all to stop
    _block_sample = std::move(block);
    _complexity = std::move(complexity);
    _is_stopping = false;
    _notification = THREAD_MESSAGE::FIND_NONCE;
    _notification_cv.notify_all();
}


void Miner::miningWorker() noexcept
{
    std::mt19937_64 mt(std::random_device{}());
    std::uniform_int_distribution<bc::NonceInt> dist(0, std::numeric_limits<bc::NonceInt>::max());

    while(true) {
        std::unique_lock lk(_notification_mutex);
        _notification_cv.wait(lk, [this] {
            return _notification != THREAD_MESSAGE::NONE;
        });

        if(_notification == THREAD_MESSAGE::EXIT) {
            break;
        }

        Block block = _block_sample;
        const base::Bytes MAX_HASH_VALUE = _complexity;

        unsigned long long iteration_counter = 0;
        while (!_is_stopping) {
            bc::NonceInt nonce = dist(mt);
            block.setNonce(nonce);

            if (++iteration_counter % 100000 == 0) {
                LOG_DEBUG << "Trying nonce " << nonce << ". Resulting hash "
                          << base::Sha256::compute(base::toBytes(block)).toHex() << ' ' << MAX_HASH_VALUE.toHex();
            }

            if (base::Sha256::compute(base::toBytes(block)).getBytes() < MAX_HASH_VALUE) {
                if (!_is_stopping) {
                    _is_stopping = true;
                    ASSERT(_callback);
                    _callback(std::move(block));
                    return;
                }
            }
        }

        if (!_is_stopping) {
            ASSERT(_callback);
            _callback({});
        }
    }
}


void Miner::setCallback(CallbackType&& callback)
{
    _callback = std::move(callback);
}


void Miner::stop()
{
    _is_stopping = true;

    _notification = THREAD_MESSAGE::EXIT;
    _notification_cv.notify_all();

    for(auto& t: _thread_pool) {
        if(t.joinable()) {
            t.join();
        }
    }
}


} // namespace bc
