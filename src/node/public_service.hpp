#pragma once

#include "core/core.hpp"
#include "core/transaction.hpp"

#include "web_socket/server.hpp"
#include "web_socket/tools.hpp"

#include <atomic>
#include <deque>
#include <functional>
#include <shared_mutex>
#include <thread>


using Task = std::function<void(void)>;


class TaskQueue
{
  public:
    TaskQueue() = default;
    ~TaskQueue() = default;
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue(TaskQueue&&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    TaskQueue& operator=(TaskQueue&&) = delete;

    void push(Task&& task);
    Task get();
    void wait();
    bool empty() const;

  private:
    mutable std::mutex _rw_mutex;
    std::deque<Task> _tasks;
    std::condition_variable _has_task;
};


class RpcService
{
  public:
    RpcService(const base::PropertyTree& config, lk::Core& core);

    ~RpcService();

    void run();
    void stop();

    void register_query(base::PropertyTree, web_socket::ResponceCall);

  private:
    const base::PropertyTree& _config;
    lk::Core& _core;
    web_socket::WebSocketServer _server;
    TaskQueue _tasks;
    std::thread _worker;

    base::PropertyTree do_route(base::PropertyTree call);
    [[noreturn]] void task_worker() noexcept;

    lk::AccountInfo getAccountInfo(const lk::Address& address);
    web_socket::NodeInfo getNodeInfo();
    lk::Block getBlock(const base::Sha256& block_hash);
    lk::Block getBlock(uint64_t block_number);
    lk::Transaction getTransaction(const base::Sha256& transaction_hash);
    lk::TransactionStatus pushTransaction(const lk::Transaction& tx);
    lk::TransactionStatus getTransactionStatus(const base::Sha256& transaction_hash);
    base::Bytes callContractView(const lk::ViewCall& call);
};
