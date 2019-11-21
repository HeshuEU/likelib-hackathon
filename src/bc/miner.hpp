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
    using CallbackType = std::function<void(std::optional<Block>)>;

    Miner();

    void findNonce(const Block& block, CallbackType&& callback);

  private:
    std::vector<std::thread> _miners_pool;

    Block _block_sample;

    CallbackType _callback;

    std::atomic<bool> _is_starting;
    std::atomic<bool> _is_stopping;
    std::atomic<bool> _is_found;

    void miningWorker() noexcept;
};

} // namespace bc
