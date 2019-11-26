#include "miner.hpp"

#include "base/assert.hpp"
#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"

#include <limits>
#include <random>

namespace bc
{


namespace impl
{

    MinerWorker::MinerWorker(const base::Bytes& complexity, Miner::CallbackType& callback) : _complexity(complexity), _callback(callback)
    {}


    MinerWorker::~MinerWorker()
    {
        stop();
    }


    void MinerWorker::run()
    {
        _thread = std::thread(&MinerWorker::threadWorker, this);
    }


    void MinerWorker::stop()
    {
        if(_thread.joinable()) {
            _thread.join();
        }
    }


    void MinerWorker::assignJob(const Block& block)
    {
        _block = block;
    }


    void MinerWorker::assignJob(Block&& block)
    {
        _block = std::move(block);
    }


    void MinerWorker::threadWorker() noexcept
    {
        while(true) {
            THREAD_MESSAGE notification;
            {
                std::unique_lock<std::mutex> lk(_notification_mutex);
                _notification_cv.wait(lk, [this]() -> bool {
                    return _has_unread_message;
                });

                notification = _notification;
            }

            if(notification == THREAD_MESSAGE::EXIT) {
                break;
            }
            else {
                ASSERT(notification == THREAD_MESSAGE::FIND_NONCE);
                while(!_has_unread_message) {
                    NonceInt nonce = _generator();

                    if(base::Sha256::compute(base::toBytes(_block)).getBytes() < _complexity) {

                    }
                }
            }
        }
    }

} // namespace impl


Miner::Miner()
{
    for(std::size_t i = 0; i < base::config::BC_MINING_THREADS; ++i) {
        // here callback is not set yet
        _workers_pool.emplace_back(_complexity, _callback);
    }
}


Miner::~Miner()
{
    stop();
}


void Miner::findNonce(const Block& block, const base::Bytes& complexity)
{
    ASSERT(_callback);
    // _block_sample = std::move(block);
    // _complexity = std::move(complexity);
}


void Miner::setCallback(CallbackType&& callback)
{
    _callback = std::move(callback);
}


void Miner::stop()
{
    for(auto& w : _workers_pool) {
        w.stop();
    }
}


} // namespace bc
