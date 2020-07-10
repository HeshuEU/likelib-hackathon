#pragma once

#include "base/utility.hpp"

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


class PublicService;


namespace tasks
{

template<typename Type>
class Queue
{
  public:
    Queue() = default;
    ~Queue() = default;
    Queue(const Queue&) = delete;
    Queue(Queue&&) = delete;
    Queue& operator=(const Queue&) = delete;
    Queue& operator=(Queue&&) = delete;

    void push(std::unique_ptr<Type>&& task);
    std::unique_ptr<Type> get();
    void wait();
    bool empty() const;

  private:
    mutable std::mutex _rw_mutex;
    std::deque<std::unique_ptr<Type>> _tasks;
    std::condition_variable _has_task;
};

class Task
{
  public:
    Task(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);
    virtual ~Task() = default;
    void run(PublicService& service);

  protected:
    websocket::SessionId _session_id;
    websocket::QueryId _query_id;
    base::PropertyTree _args;

    virtual bool prepareArgs() = 0;
    virtual void execute(PublicService& service) = 0;
};


class FindBlockTask final : public Task
{
  public:
    FindBlockTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;

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
    void execute(PublicService& service) override;

  private:
    std::optional<base::Sha256> _tx_hash;
};


class PushTransactionTask final : public Task
{
  public:
    PushTransactionTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;

  private:
    std::optional<lk::Transaction> _tx;
    std::optional<base::Sha256> _tx_hash;
};


class FindTransactionStatusTask final : public Task
{
  public:
    FindTransactionStatusTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;

  private:
    std::optional<base::Sha256> _tx_hash;
};


class NodeInfoCallTask final : public Task
{
  public:
    NodeInfoCallTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;
};


class NodeInfoSubscribeTask final : public Task
{
  public:
    NodeInfoSubscribeTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;
};


class NodeInfoUnsubscribeTask final : public Task
{
  public:
    NodeInfoUnsubscribeTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;
};


class AccountInfoCallTask final : public Task
{
  public:
    AccountInfoCallTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;

  private:
    std::optional<lk::Address> _address;
};


class AccountInfoSubscribeTask final : public Task
{
  public:
    AccountInfoSubscribeTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;

  private:
    std::optional<lk::Address> _address;
};


class AccountInfoUnsubscribeTask final : public Task
{
  public:
    AccountInfoUnsubscribeTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;

  private:
    std::optional<lk::Address> _address;
};


class UnsubscribeTransactionStatusUpdateTask final : public Task
{
  public:
    UnsubscribeTransactionStatusUpdateTask(websocket::SessionId session_id,
                                           websocket::QueryId query_id,
                                           base::PropertyTree&& args);

  protected:
    bool prepareArgs() override;
    void execute(PublicService& service) override;

  private:
    std::optional<base::Sha256> _tx_hash;
};

}

class PublicService
{
    friend tasks::FindBlockTask;
    friend tasks::FindTransactionTask;
    friend tasks::FindTransactionStatusTask;
    friend tasks::NodeInfoCallTask;
    friend tasks::NodeInfoSubscribeTask;
    friend tasks::NodeInfoUnsubscribeTask;
    friend tasks::AccountInfoCallTask;
    friend tasks::PushTransactionTask;
    friend tasks::AccountInfoSubscribeTask;
    friend tasks::AccountInfoUnsubscribeTask;
    friend tasks::UnsubscribeTransactionStatusUpdateTask;

  public:
    PublicService(const base::PropertyTree& config, lk::Core& core);

    ~PublicService();

    void run();
    void stop();

  private:
    const base::PropertyTree& _config;
    lk::Core& _core;
    websocket::WebSocketAcceptor _acceptor;

    websocket::SessionId _last_given_session_id{ 0 };
    std::unordered_map<websocket::SessionId, std::unique_ptr<websocket::WebSocketSession>> _running_sessions;

    tasks::Queue<tasks::Task> _input_tasks;
    std::thread _worker;

    base::Observable<base::Sha256> _event_transaction_status_update;
    std::unordered_map<websocket::SessionId, std::unordered_map<base::Sha256, std::size_t>>
      _transaction_status_update_sessions_registry;

    base::Observable<const lk::ImmutableBlock&> _event_block_added;
    std::unordered_map<websocket::SessionId, std::size_t> _info_update_sessions_registry;

    base::Observable<lk::Address> _event_account_update;
    std::unordered_map<websocket::SessionId, std::unordered_map<lk::Address, std::size_t>>
      _account_update_sessions_registry;

    void createSession(boost::asio::ip::tcp::socket&& socket);
    websocket::SessionId createId();

    void on_session_request(websocket::SessionId session_id,
                            websocket::QueryId query_id,
                            websocket::Command::Id command_id,
                            base::PropertyTree&& args);
    void on_session_close(websocket::SessionId session_id);

    [[noreturn]] void task_worker() noexcept;

    void sendResponse(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& result);

    void on_added_new_block(const lk::ImmutableBlock& block);
    void on_update_transaction_status(base::Sha256 tx_hash);
    void on_update_account(lk::Address account_address);
};

#include "public_service.tpp"