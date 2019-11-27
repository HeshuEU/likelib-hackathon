#include "miner.hpp"

#include "base/assert.hpp"
#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"

#include <limits>
#include <utility>

namespace bc
{


namespace impl
{

    MinerWorker::MinerWorker(const base::Bytes& complexity, Miner::CallbackType& callback)
        : _complexity(complexity), _callback(callback)
    {}


    MinerWorker::~MinerWorker()
    {
        stop();
    }


    void MinerWorker::run()
    {
        _thread = std::move(std::thread(&MinerWorker::threadWorker, this));
    }


    void MinerWorker::stop()
    {
        {
            std::lock_guard lk(_notification_mutex);
            _has_unread_message = true;
            _notification = THREAD_MESSAGE::EXIT;
            _notification_cv.notify_one();
        }
        if(_thread.joinable()) {
            _thread.join();
        }
    }


    void MinerWorker::assignJob(const Block& block)
    {
        {
            std::lock_guard lk(_notification_mutex);
            _has_unread_message = true;
            _notification = THREAD_MESSAGE::FIND_NONCE;
            _block = block;
            _notification_cv.notify_one();
        }
    }


    void MinerWorker::assignJob(Block&& block)
    {
        {
            std::lock_guard lk(_notification_mutex);
            _has_unread_message = true;
            _notification = THREAD_MESSAGE::FIND_NONCE;
            _block = std::move(block);
            _notification_cv.notify_one();
        }
    }


    void MinerWorker::dropJob()
    {
        {
            std::lock_guard lk(_notification_mutex);
            _notification = THREAD_MESSAGE::NONE;
            _has_unread_message = true;
            _notification_cv.notify_one();
        }
    }


    void MinerWorker::threadWorker() noexcept
    {
        Block block;
        while(true) {
            THREAD_MESSAGE notification;
            {
                std::unique_lock<std::mutex> lk(_notification_mutex);
                _notification_cv.wait(lk, [this]() -> bool {
                    return _has_unread_message && _notification != THREAD_MESSAGE::NONE;
                });

                notification = _notification;
                if(notification == THREAD_MESSAGE::FIND_NONCE) {
                    block = std::move(_block);
                }
                _has_unread_message = false;
            }

            if(notification == THREAD_MESSAGE::EXIT) {
                break;
            }
            else {
                ASSERT(notification == THREAD_MESSAGE::FIND_NONCE);
                while(!_has_unread_message) {
                    NonceInt nonce = _generator();
                    block.setNonce(nonce);
                    if(base::Sha256::compute(base::toBytes(block)).getBytes() < _complexity) {
                        _callback(std::move(block));
                        break;
                    }
                }
            }
        }
    }

} // namespace impl



namespace
{

    std::size_t calcMiningThreadsNum(const base::PropertyTree& ptree)
    {
        if(ptree.hasKey("miner.threads")) {
            return ptree.get<std::size_t>("miner.threads");
        }
        else {
            std::size_t ret = std::thread::hardware_concurrency();
            if(ret <= 2) {
                ret = 2;
            }
            return ret;
        }
    }

} // namespace


Miner::Miner(const base::PropertyTree& ptree)
{
    const std::size_t numThreads = calcMiningThreadsNum(ptree);
    for(std::size_t i = 0; i < numThreads; ++i) {
        _workers_pool.emplace_front(_complexity, _callback); // here callback is not set yet
        _workers_pool.front().run();
    }
    LOG_INFO << "Miner is running on " << numThreads << " threads";
}


Miner::~Miner()
{
    stop();
}


void Miner::findNonce(const Block& block, const base::Bytes& complexity)
{
    ASSERT(_callback);
    _complexity = complexity;
    for(auto& w: _workers_pool) {
        w.assignJob(block);
    }
}


void Miner::setCallback(CallbackType&& callback)
{
    _callback = [callback = std::move(callback), this](Block&& block) {
        dropJob();
        callback(std::move(block));
    };
}


void Miner::dropJob()
{
    for(auto& w: _workers_pool) {
        w.dropJob();
    }
}


void Miner::stop()
{
    for(auto& w: _workers_pool) {
        w.stop();
    }
}


} // namespace bc
