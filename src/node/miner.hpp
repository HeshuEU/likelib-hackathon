#pragma once

#include "base/bytes.hpp"
#include "base/property_tree.hpp"
#include "core/block.hpp"
#include "core/consensus.hpp"
#include "core/types.hpp"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <forward_list>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>

namespace impl
{
class MinerWorker;

using MinerHandlerType = std::function<void(lk::ImmutableBlock&&)>;

enum class Task
{
    NONE,
    DROP_JOB,
    FIND_NONCE,
    EXIT
};


struct CommonData
{
    impl::Task task;
    std::optional<lk::MutableBlock> block_to_mine;
    std::optional<lk::Complexity> complexity;
};


class CommonState
{
  public:
    //===================
    CommonState(CommonData&& initial_state, MinerHandlerType handler);
    //===================
    std::size_t getVersion() const;
    //===================
    [[maybe_unused]] std::size_t getCommonData(CommonData& data) const;
    void setCommonData(const CommonData& data);
    //===================
    template<typename... Args>
    void callHandlerAndDrop(Args&&... args);
    //===================
    void waitAndReadNewData(std::size_t& last_read_version, CommonData& data);
    //===================
  private:
    //===================
    mutable std::shared_mutex _state_mutex;
    std::condition_variable_any _state_changed_cv;
    std::atomic<std::size_t> _version;
    //===================
    CommonData _common_data;
    MinerHandlerType _handler;
    //===================
};

} // namespace impl


class Miner
{
  public:
    //===================
    using HandlerType = impl::MinerHandlerType;
    //===================
    Miner(const base::PropertyTree& config, HandlerType handler);

    ~Miner();
    //===================
    void findNonce(const lk::MutableBlock& block_without_nonce, const lk::Complexity& complexity);
    void dropJob();
    //===================
  private:
    //===================
    impl::CommonState _common_state;
    //===================
    std::forward_list<impl::MinerWorker> _workers;
    //===================
    void stop();
    //===================
};
