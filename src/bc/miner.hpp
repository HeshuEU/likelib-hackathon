#pragma once

#include "bc/types.hpp"
#include "bc/block.hpp"

#include <atomic>
#include <cstddef>
#include <functional>
#include <thread>


namespace bc
{
class Miner
{
  public:
    //================
    Miner();
    ~Miner();
    //================
    using CallbackType = std::function<void(std::optional<Block>)>;

    void setCallback(CallbackType&& callback);

    void findNonce(const Block& block);
    void stop();
    //================
  private:
    //================
    std::vector<std::thread> _thread_pool;

    Block _block_sample;
    CallbackType _callback;
    std::atomic<bool> _is_stopping;
    //================
    void miningWorker() noexcept;
    //================
};

} // namespace bc
