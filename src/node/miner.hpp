#pragma once

#include "base/bytes.hpp"
#include "base/property_tree.hpp"
#include "lk/block.hpp"
#include "lk/types.hpp"

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

using MinerHandlerType = std::function<void(lk::Block&&)>;

enum class Task
{
    NONE,
    DROP_JOB,
    FIND_NONCE,
    EXIT
};


struct CommonData
{
    static constexpr std::size_t COMPLEXITY_SIZE = 32;
    impl::Task task;
    std::optional<lk::Block> block_to_mine;
    std::optional<base::FixedBytes<COMPLEXITY_SIZE>> complexity;
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
    void findNonce(const lk::Block& block_without_nonce,
                   const base::FixedBytes<impl::CommonData::COMPLEXITY_SIZE>& complexity);
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
