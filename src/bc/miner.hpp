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
    class MinerWorker;

    enum class Task
    {
        NONE,
        DROP_JOB,
        FIND_NONCE,
        EXIT
    };
} // namespace impl


class Miner
{
  public:
    //===================
    using HandlerType = std::function<void(Block&&)>;
    //===================
    Miner(const base::PropertyTree& config, HandlerType handler);

    ~Miner();
    //===================
    void findNonce(const Block& block_without_nonce);
    void dropJob();
    //===================
  private:
    //===================
    HandlerType _handler;
    //===================
    std::forward_list<impl::MinerWorker> _workers;
    //===================
    impl::Task _task;
    std::optional<Block> _block_to_mine;
    std::atomic<std::size_t> _job_version;
    mutable std::shared_mutex _state_mutex;
    std::condition_variable_any _state_changed_cv;
    //===================
    void stop();
    //===================
};



} // namespace bc
