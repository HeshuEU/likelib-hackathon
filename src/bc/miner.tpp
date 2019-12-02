#pragma once

#include "miner.hpp"

#include <random>
#include <utility>

namespace bc
{

template<typename H>
Miner<H>::Miner(H handler) : _handler{handler}
{
    // setting up initial state
    _task = impl::Task::NONE;
    _job_version = 0;

    // setting up threads
    for(std::size_t i = 0; i < getThreadsNumber(); ++i) {
        _workers.emplace_front(_job_version, _task, _block_to_mine, _state_mutex, _state_changed_cv, _handler);
    }
}


template<typename H>
Miner<H>::~Miner()
{
    stop();
}


template<typename H>
std::size_t Miner<H>::getThreadsNumber()
{
    return std::thread::hardware_concurrency();
}


template<typename H>
void Miner<H>::findNonce(const Block& block_without_nonce)
{
    std::unique_lock lk(_state_mutex);
    ++_job_version;
    _task = impl::Task::FIND_NONCE;
    _block_to_mine = block_without_nonce;
    _state_changed_cv.notify_all();
}


template<typename H>
void Miner<H>::dropJob()
{
    std::unique_lock lk(_state_mutex);
    ++_job_version;
    _task = impl::Task::NONE;
    _block_to_mine.reset();
    _state_changed_cv.notify_all();
}


template<typename H>
void Miner<H>::stop()
{
    std::unique_lock lk(_state_mutex);
    ++_job_version;
    _task = impl::Task::EXIT;
    _block_to_mine.reset();
    _state_changed_cv.notify_all();
}

//=============================

namespace impl
{

    inline base::Bytes tempGetComplexity()
    {
        // TODO: rewrite
        base::Bytes b(32);
        b[2] = 0x7f;
        return b;
    }

    template<typename Handler>
    MinerWorker<Handler>::MinerWorker(std::atomic<std::size_t>& version, impl::Task& task,
        std::optional<Block>& block_to_mine, std::shared_mutex& state_mutex,
        std::condition_variable_any& state_changed_cv, Handler handler)
        : _last_read_version{0}, _version{version}, _task{task}, _block_to_mine{block_to_mine},
          _state_mutex{state_mutex}, _state_changed_cv{state_changed_cv}, _handler{handler}
    {
        _worker_thread = std::thread(&MinerWorker<Handler>::worker, this);
    }


    template<typename Handler>
    MinerWorker<Handler>::~MinerWorker()
    {
        if(_worker_thread.joinable()) {
            _worker_thread.join();
        }
    }


    template<typename Handler>
    void MinerWorker<Handler>::worker() noexcept
    {
        bool is_stopping{false};
        std::mt19937_64 mt{std::random_device{}()};

        while(!is_stopping) {
            {
                std::shared_lock lk(_state_mutex);
                _state_changed_cv.wait(lk, [this] {
                    return _last_read_version != _version;
                });
                _last_read_version = _version;
                _last_read_task = _task;
                _last_read_block_to_mine = _block_to_mine;
            }

            switch(_last_read_task) {
                case Task::NONE: {
                    // do nothing
                    break;
                }
                case Task::EXIT: {
                    is_stopping = true;
                    break;
                }
                case Task::DROP_JOB: {
                    // do nothing, the job is already dropped
                    break;
                }
                case Task::FIND_NONCE: {
                    ASSERT(_last_read_block_to_mine);
                    Block& b = _last_read_block_to_mine.value();
                    while(_last_read_version == _version) {
                        auto attempting_nonce = mt();
                        b.setNonce(attempting_nonce);
                        if(base::toBytes(b) < tempGetComplexity()) {
                            std::unique_lock lk(_state_mutex);
                            _handler(b);
                            _version++;
                            _task = Task::DROP_JOB;
                            _block_to_mine.reset();
                            _state_changed_cv.notify_all();
                        }
                    }
                    break;
                }
                default: {
                    ASSERT(false);
                    break;
                }
            }
        }
    }

} // namespace impl

} // namespace bc