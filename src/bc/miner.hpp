#pragma once

#include "base/bytes.hpp"
#include "base/property_tree.hpp"
#include "bc/types.hpp"
#include "bc/block.hpp"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <random>
#include <thread>
#include <forward_list>


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
    Miner(const base::PropertyTree& ptree);
    ~Miner();
    //================
    using CallbackType = std::function<void(Block&&)>;

    void setCallback(CallbackType&& callback);

    void findNonce(const Block& block, const base::Bytes& mining_complexity);
    void dropJob();
    //================
  private:
    //================
    std::forward_list<impl::MinerWorker> _workers_pool;
    //================
    CallbackType _callback;
    base::Bytes _complexity;
    //================
    void miningWorker() noexcept;
    //================
    void stop();
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
        void dropJob();
        //==================
      private:
        //==================
        std::thread _thread;
        Block _block;
        Miner::CallbackType& _callback;
        const base::Bytes& _complexity;
        //==================
        std::atomic<bool> _has_unread_message{false};
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
        std::mt19937_64 _generator{std::random_device{}()};
        //==================
    };
} // namespace impl

} // namespace bc
