#pragma once

#include "base/bytes.hpp"
#include "base/property_tree.hpp"
#include "bc/types.hpp"
#include "bc/block.hpp"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <forward_list>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>


namespace bc
{

namespace impl
{
    enum class Task
    {
        NONE,
        DROP_JOB,
        FIND_NONCE,
        EXIT
    };

    template<typename Handler>
    class MinerWorker
    {
      public:
        //===================
        MinerWorker(std::atomic<std::size_t>& version, Task& task, std::optional<Block>& block_to_mine,
            std::shared_mutex& state_mutex, std::condition_variable_any& state_changed_cv, Handler handler);

        ~MinerWorker();
        //===================
      private:
        //===================
        std::thread _worker_thread;
        std::size_t _last_read_version;
        Task _last_read_task;
        std::optional<Block> _last_read_block_to_mine;
        //===================
        std::atomic<std::size_t>& _version;
        Task& _task;
        std::optional<Block>& _block_to_mine;
        std::shared_mutex& _state_mutex;
        std::condition_variable_any& _state_changed_cv;
        Handler _handler;
        //===================
        void worker() noexcept;
        //===================
    };

    template<typename Handler>
    MinerWorker(std::atomic<std::size_t>, impl::Task&, std::optional<Block>&, std::shared_mutex&,
        std::condition_variable_any&, Handler&)
        ->MinerWorker<Handler>;

} // namespace impl

template<typename H>
class Miner
{
  public:
    //===================
    Miner(H handler);

    ~Miner();
    //===================
    void findNonce(const Block& block_without_nonce);
    void dropJob();
    //===================
  private:
    //===================
    H _handler;
    //===================
    std::forward_list<impl::MinerWorker<H>> _workers;
    //===================
    impl::Task _task;
    std::optional<Block> _block_to_mine;
    std::atomic<std::size_t> _job_version;
    mutable std::shared_mutex _state_mutex;
    std::condition_variable_any _state_changed_cv;
    //===================
    static std::size_t getThreadsNumber();
    void stop();
    //===================
};

template<typename H>
Miner(H &&)->Miner<H>;

} // namespace bc

#include "miner.tpp"
