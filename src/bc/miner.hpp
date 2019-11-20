#pragma once

#include "bc/types.hpp"
#include "bc/block.hpp"

#include <atomic>
#include <cstddef>
#include <future>
#include <optional>
#include <thread>


namespace bc
{
class Miner
{
  public:
    Miner();

    std::future<std::optional<NonceInt>> findNonce(const Block& block);

  private:
    std::vector<std::thread> _miners_pool;

    // Block _block_sample;

    void miningWorker() noexcept;
};

} // namespace bc
