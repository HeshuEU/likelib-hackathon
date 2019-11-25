#pragma once

#include "base/bytes.hpp"
#include "bc/types.hpp"
#include "bc/block.hpp"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
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

    void findNonce(const Block& block, const base::Bytes& mining_complexity);
    void stop();
    //================
  private:
    //================
    std::vector<std::thread> _thread_pool;
    std::condition_variable _notification_cv;
    std::mutex _notification_mutex;

    enum class THREAD_MESSAGE {
        NONE,
        FIND_NONCE,
        EXIT
    };
    std::atomic<THREAD_MESSAGE> _notification;
    //================
    Block _block_sample;
    CallbackType _callback;
    std::atomic<bool> _is_stopping;
    base::Bytes _complexity;
    //================
    void miningWorker() noexcept;
    //================
};

} // namespace bc
