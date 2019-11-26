#pragma once

#include "base/bytes.hpp"
#include "bc/types.hpp"
#include "bc/block.hpp"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <random>
#include <thread>


namespace bc
{

namespace impl
{
    class MinerWorker;
}


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
    std::vector<impl::MinerWorker> _workers_pool;
    //================
    Block _block_sample;
    CallbackType _callback;
    base::Bytes _complexity;
    //================
    void miningWorker() noexcept;
    //================
};


namespace impl
{
    class MinerWorker
    {
      public:
        //==================
        MinerWorker(const base::Bytes& complexity, Miner::CallbackType& callback);
        ~MinerWorker();
        //==================
        void run();
        void stop();
        //==================
        void assignJob(const Block& block);
        void assignJob(Block&& block);
        //==================
      private:
        //==================
        std::thread _thread;
        Block _block;
        Miner::CallbackType& _callback;
        const base::Bytes& _complexity;
        //==================
        std::atomic<bool> _has_unread_message;
        std::condition_variable _notification_cv;
        std::mutex _notification_mutex;

        enum class THREAD_MESSAGE
        {
            NONE,
            FIND_NONCE,
            EXIT
        };
        std::atomic<THREAD_MESSAGE> _notification{THREAD_MESSAGE::NONE};

        void threadWorker() noexcept;
        //==================
        std::mt19937_64 _generator{ std::random_device{}() };
        //==================
    };
} // namespace impl

} // namespace bc
