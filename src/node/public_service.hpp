#pragma once

#include "core/core.hpp"
#include "core/transaction.hpp"

#include "websocket/acceptor.hpp"
#include "websocket/session.hpp"
#include "websocket/tools.hpp"

#include "base/utility.hpp"

#include <atomic>
#include <functional>
#include <thread>


class PublicService;

namespace tasks
{

class Task
{
  public:
    Task(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);
    virtual ~Task() = default;
    void run(PublicService& service);
    virtual const std::string& name() const noexcept = 0;

  protected:
    websocket::SessionId _session_id;
    websocket::QueryId _query_id;
    base::json::Value _args;

    virtual void prepareArgs() = 0;
    virtual void execute(PublicService& service) = 0;
};


class FindBlockTask final : public Task
{
  public:
    FindBlockTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<base::Sha256> _block_hash;
    std::optional<std::uint64_t> _block_depth;
};


class FindTransactionTask final : public Task
{
  public:
    FindTransactionTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<base::Sha256> _tx_hash;
};


class PushTransactionTask final : public Task
{
  public:
    PushTransactionTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<lk::Transaction> _tx;
    std::optional<base::Sha256> _tx_hash;
};


class FindTransactionStatusTask final : public Task
{
  public:
    FindTransactionStatusTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<base::Sha256> _tx_hash;
};


class NodeInfoCallTask final : public Task
{
  public:
    NodeInfoCallTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;
};


class NodeInfoSubscribeTask final : public Task
{
  public:
    NodeInfoSubscribeTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;
};


class NodeInfoUnsubscribeTask final : public Task
{
  public:
    NodeInfoUnsubscribeTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;
};


class AccountInfoCallTask final : public Task
{
  public:
    AccountInfoCallTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<lk::Address> _address;
};


class FeeInfoCallTask final : public Task
{
  public:
    FeeInfoCallTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<lk::Address> _address;
};


class AccountInfoSubscribeTask final : public Task
{
  public:
    AccountInfoSubscribeTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<lk::Address> _address;
};


class AccountInfoUnsubscribeTask final : public Task
{
  public:
    AccountInfoUnsubscribeTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<lk::Address> _address;
};


class UnsubscribeTransactionStatusUpdateTask final : public Task
{
  public:
    UnsubscribeTransactionStatusUpdateTask(websocket::SessionId session_id,
                                           websocket::QueryId query_id,
                                           base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::optional<base::Sha256> _tx_hash;
};


class LoginTask final : public Task
{
  public:
    LoginTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args);

  protected:
    void prepareArgs() override;
    void execute(PublicService& service) override;
    const std::string& name() const noexcept override;

  private:
    std::string _login;
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
    friend tasks::FeeInfoCallTask;
    friend tasks::PushTransactionTask;
    friend tasks::AccountInfoSubscribeTask;
    friend tasks::AccountInfoUnsubscribeTask;
    friend tasks::UnsubscribeTransactionStatusUpdateTask;
    friend tasks::LoginTask;

  public:
    PublicService(base::json::Value config, lk::Core& core);

    ~PublicService();

    void run();
    void stop();

    void sendCorrectResponse(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& result);
    void sendErrorResponse(websocket::SessionId session_id,
                           websocket::QueryId query_id,
                           const std::string& error_message);

  private:
    lk::Core& _core;

    websocket::SessionId _last_given_session_id{ 0 };
    std::unordered_map<websocket::SessionId, std::unique_ptr<websocket::WebSocketSession>> _running_sessions;

    base::Queue<tasks::Task> _input_tasks;
    std::thread _worker;

    base::Observable<base::Sha256> _event_transaction_status_update;
    std::unordered_map<websocket::SessionId, std::unordered_map<base::Sha256, std::size_t>>
      _transaction_status_update_sessions_registry;

    base::Observable<const lk::ImmutableBlock&> _event_block_added;
    std::unordered_map<websocket::SessionId, std::size_t> _info_update_sessions_registry;

    base::Observable<lk::Address> _event_account_update;
    std::unordered_map<websocket::SessionId, std::unordered_map<lk::Address, std::size_t>>
      _account_update_sessions_registry;

    std::string _login;
    std::vector<websocket::SessionId> _admins_sessions;

    websocket::WebSocketAcceptor _acceptor;

    void createSession(boost::asio::ip::tcp::socket&& socket);
    websocket::SessionId createId();

    void on_session_request(websocket::SessionId session_id,
                            websocket::QueryId query_id,
                            websocket::Command::Id command_id,
                            base::json::Value&& args);
    void on_session_close(websocket::SessionId session_id);

    [[noreturn]] void task_worker() noexcept;


    void on_added_new_block(const lk::ImmutableBlock& block);
    void on_updated_transaction_status(base::Sha256 tx_hash);
    void on_update_account(lk::Address account_address);

    void addAdminSession(const websocket::SessionId& id);
};
