#pragma once

#include "core/core.hpp"
#include "core/transaction.hpp"

#include "websocket/acceptor.hpp"
#include "websocket/session.hpp"
#include "websocket/tools.hpp"

#include <atomic>
#include <deque>
#include <functional>
#include <shared_mutex>
#include <thread>

namespace tasks
{

class Task;

class TaskQueue
{
  public:
    TaskQueue() = default;
    ~TaskQueue() = default;
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue(TaskQueue&&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    TaskQueue& operator=(TaskQueue&&) = delete;

    void push(std::unique_ptr<Task>&& task);
    std::unique_ptr<Task> get();
    void wait();
    bool empty() const;

  private:
    mutable std::mutex _rw_mutex;
    std::deque<std::unique_ptr<Task>> _tasks;
    std::condition_variable _has_task;
};


using SendResponse = std::function<void(websocket::SessionId, websocket::QueryId, base::PropertyTree&&)>;


class Task
{
  public:
    Task(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);
    virtual ~Task() = default;
    void run(lk::Core& core, SendResponse send_response);

  protected:
    websocket::SessionId _session_id;
    websocket::QueryId _query_id;
    base::PropertyTree _args;

    virtual bool prepareArgs() = 0;
    virtual void execute(lk::Core& core, SendResponse send_response) = 0;
};


class ViewCallTask final : public Task
{
  public:
    ViewCallTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
    std::optional<lk::ViewCall> _view_call;
};


class FindBlockTask final : public Task
{
  public:
    FindBlockTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
    std::optional<base::Sha256> _block_hash;
    std::optional<std::uint64_t> _block_number;
};


class FindTransactionTask final : public Task
{
  public:
    FindTransactionTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
    std::optional<base::Sha256> _tx_hash;
};


class FindTransactionStatusTask final : public Task
{
  public:
    FindTransactionStatusTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
    std::optional<base::Sha256> _tx_hash;
};


class NodeInfoCallTask final : public Task
{
  public:
    NodeInfoCallTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;
};


class AccountInfoCallTask final : public Task
{
  public:
    AccountInfoCallTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
    std::optional<lk::Address> _address;
};


class PushTransactionSubscriptionTask final : public Task
{
  public:
    PushTransactionSubscriptionTask(websocket::SessionId session_id,
                                    websocket::QueryId query_id,
                                    base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
};


class NodeInfoSubscriptionTask final : public Task
{
  public:
    NodeInfoSubscriptionTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
};


class AccountInfoSubscriptionTask final : public Task
{
  public:
    AccountInfoSubscriptionTask(websocket::SessionId session_id,
                                websocket::QueryId query_id,
                                base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
    std::optional<lk::Address> _address;
};


class PushTransactionUnsubscriptionTask final : public Task
{
  public:
    PushTransactionUnsubscriptionTask(websocket::SessionId session_id,
                                      websocket::QueryId query_id,
                                      base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
};


class NodeInfoUnsubscriptionTask final : public Task
{
  public:
    NodeInfoUnsubscriptionTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
};


class AccountInfoUnsubscriptionTask final : public Task
{
  public:
    AccountInfoUnsubscriptionTask(websocket::SessionId session_id,
                                  websocket::QueryId query_id,
                                  base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(lk::Core& core, SendResponse send_response) override;

  private:
    std::optional<lk::Address> _address;
};

}

class RpcService
{
  public:
    RpcService(const base::PropertyTree& config, lk::Core& core);

    ~RpcService();

    void run();
    void stop();

  private:
    const base::PropertyTree& _config;
    lk::Core& _core;
    websocket::WebSocketAcceptor _acceptor;

    websocket::SessionId _last_session_id{ 0 };
    std::unordered_map<websocket::SessionId, std::unique_ptr<websocket::WebSocketSession>> _sessions_map;

    tasks::TaskQueue _tasks;
    std::thread _worker;

    void createSession(boost::asio::ip::tcp::socket&& socket);
    websocket::SessionId createId();

    void on_session_request(websocket::SessionId session_id,
                             websocket::QueryId query_id,
                             websocket::Command::Id command_id,
                             base::PropertyTree&& args);
    void on_session_close(websocket::SessionId session_id);

    [[noreturn]] void task_worker() noexcept;

    void on_send_response(websocket::SessionId, websocket::QueryId, base::PropertyTree&&);
};
