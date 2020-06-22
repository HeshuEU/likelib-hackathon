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

    using Task = std::function<void(void)>;
    std::condition_variable _has_task;
    std::mutex _tasks_mutex;
    std::deque<Task> _tasks;
    std::thread _worker;

    base::PropertyTree do_route(base::PropertyTree call);
    [[noreturn]] void task_worker();

    lk::AccountInfo getAccountInfo(const lk::Address& address);
    web_socket::NodeInfo getNodeInfo();
    lk::Block getBlock(const base::Sha256& block_hash);
    lk::Block getBlock(uint64_t block_number);
    lk::Transaction getTransaction(const base::Sha256& transaction_hash);
    lk::TransactionStatus pushTransaction(const lk::Transaction& tx);
    lk::TransactionStatus getTransactionStatus(const base::Sha256& transaction_hash);
    base::Bytes callContractView(const lk::ViewCall& call);
};
