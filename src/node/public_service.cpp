#include "public_service.hpp"

#include "core/transaction.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"

namespace tasks
{

void TaskQueue::push(std::unique_ptr<Task>&& task)
{
    {
        std::lock_guard lock(_rw_mutex);
        _tasks.emplace_back(std::move(task));
    }
    _has_task.notify_one();
}


std::unique_ptr<Task> TaskQueue::get()
{
    std::lock_guard lock(_rw_mutex);
    std::unique_ptr<Task> current_task{ _tasks.front().release() };
    _tasks.pop_front();
    return current_task;
}


void TaskQueue::wait()
{
    std::unique_lock lock(_rw_mutex);
    _has_task.wait(lock, [this]() { return !_tasks.empty(); });
}


bool TaskQueue::empty() const
{
    std::lock_guard lk(_rw_mutex);
    return _tasks.empty();
}


Task::Task(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args)
  : _session_id{ session_id }
  , _query_id{ query_id }
  , _args{ std::move(args) }
{}


void Task::run(lk::Core& core, SendResponse send_response)
{
    ASSERT(send_response);
    if (prepareArgs()) {
        return execute(core, std::move(send_response));
    }
    LOG_DEBUG << "fail to prepare arguments to execution";
    RAISE_ERROR(base::InvalidArgument, "Bad command arguments");
}


FindBlockTask::FindBlockTask(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool FindBlockTask::prepareArgs()
{
    if (_args.hasKey("hash")) {
        _block_hash = websocket::deserializeHash(_args.get<std::string>("hash"));
        if (!_block_hash) {
            LOG_DEBUG << "deserialization error";
            return false;
        }
        return true;
    }
    else if (_args.hasKey("number")) {
        _block_number = _args.get<std::uint64_t>("number");
        return true;
    }
    else {
        LOG_DEBUG << "not any options exists";
        return false;
    }
}


void FindBlockTask::execute(lk::Core& core, SendResponse send_response)
{
    ASSERT(send_response);
    if (_block_hash) {
        auto block = core.findBlock(_block_hash.value());
        if (block) {
            base::PropertyTree answer = websocket::serializeBlock(block.value());
            send_response(_session_id, _query_id, std::move(answer));
        }
    }
    else {
        auto block_hash = core.findBlockHash(_block_number.value());
        if (block_hash) {
            auto block = core.findBlock(block_hash.value());
            base::PropertyTree answer = websocket::serializeBlock(block.value());
            send_response(_session_id, _query_id, std::move(answer));
        }
    }
}


FindTransactionTask::FindTransactionTask(websocket::SessionId session_id,
                                         websocket::QueryId query_id,
                                         base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool FindTransactionTask::prepareArgs()
{
    if (_args.hasKey("hash")) {
        _tx_hash = websocket::deserializeHash(_args.get<std::string>("hash"));
        if (!_tx_hash) {
            LOG_DEBUG << "deserialization error";
            return false;
        }
        return true;
    }
    LOG_DEBUG << "not any options exists";
    return false;
}


void FindTransactionTask::execute(lk::Core& core, SendResponse send_response)
{
    ASSERT(send_response);
    auto tx = core.findTransaction(_tx_hash.value());
    base::PropertyTree answer = websocket::serializeTransaction(tx.value());
    send_response(_session_id, _query_id, std::move(answer));
}


FindTransactionStatusTask::FindTransactionStatusTask(websocket::SessionId session_id,
                                                     websocket::QueryId query_id,
                                                     base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool FindTransactionStatusTask::prepareArgs()
{
    if (_args.hasKey("hash")) {
        _tx_hash = websocket::deserializeHash(_args.get<std::string>("hash"));
        if (!_tx_hash) {
            LOG_DEBUG << "deserialization error";
            return false;
        }
        return true;
    }
    LOG_DEBUG << "not any options exists";
    return false;
}


void FindTransactionStatusTask::execute(lk::Core& core, SendResponse send_response)
{
    ASSERT(send_response);
    auto tx_status = core.getTransactionOutput(_tx_hash.value());
    if (tx_status) {
        base::PropertyTree answer = websocket::serializeTransactionStatus(tx_status.value());
        send_response(_session_id, _query_id, std::move(answer));
        return;
    }
    LOG_DEBUG << "Cant find transaction status task";
}

NodeInfoCallTask::NodeInfoCallTask(websocket::SessionId session_id,
                                   websocket::QueryId query_id,
                                   base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool NodeInfoCallTask::prepareArgs()
{
    return true;
}


void NodeInfoCallTask::execute(lk::Core& core, SendResponse send_response)
{
    ASSERT(send_response);
    auto last_block_hash = core.getTopBlockHash();
    auto last_block_number = core.getTopBlock().getDepth();
    websocket::NodeInfo info{ last_block_hash, last_block_number };
    base::PropertyTree answer = websocket::serializeInfo(info);
    send_response(_session_id, _query_id, std::move(answer));
}


AccountInfoCallTask::AccountInfoCallTask(websocket::SessionId session_id,
                                         websocket::QueryId query_id,
                                         base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool AccountInfoCallTask::prepareArgs()
{
    if (_args.hasKey("address")) {
        _address = websocket::deserializeAddress(_args.get<std::string>("address"));
        if (!_address) {
            LOG_DEBUG << "deserialization error";
            return false;
        }
        return true;
    }
    LOG_DEBUG << "not any options exists";
    return false;
}


void AccountInfoCallTask::execute(lk::Core& core, SendResponse send_response)
{
    ASSERT(send_response);
    auto account_info = core.getAccountInfo(_address.value());
    base::PropertyTree answer = websocket::serializeAccountInfo(account_info);
    send_response(_session_id, _query_id, std::move(answer));
}


PushTransactionSubscriptionTask::PushTransactionSubscriptionTask(websocket::SessionId session_id,
                                                                 websocket::QueryId query_id,
                                                                 base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool PushTransactionSubscriptionTask::prepareArgs()
{
    // TODO
}


void PushTransactionSubscriptionTask::execute(lk::Core& core, SendResponse send_response)
{
    // TODO
}


NodeInfoSubscriptionTask::NodeInfoSubscriptionTask(websocket::SessionId session_id,
                                                   websocket::QueryId query_id,
                                                   base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool NodeInfoSubscriptionTask::prepareArgs()
{
    // TODO
}


void NodeInfoSubscriptionTask::execute(lk::Core& core, SendResponse send_response)
{
    // TODO
}


AccountInfoSubscriptionTask::AccountInfoSubscriptionTask(websocket::SessionId session_id,
                                                         websocket::QueryId query_id,
                                                         base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool AccountInfoSubscriptionTask::prepareArgs()
{
    // TODO
}


void AccountInfoSubscriptionTask::execute(lk::Core& core, SendResponse send_response)
{
    // TODO
}

PushTransactionUnsubscriptionTask::PushTransactionUnsubscriptionTask(websocket::SessionId session_id,
                                                                     websocket::QueryId query_id,
                                                                     base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool PushTransactionUnsubscriptionTask::prepareArgs()
{
    // TODO
}


void PushTransactionUnsubscriptionTask::execute(lk::Core& core, SendResponse send_response)
{
    // TODO
}


NodeInfoUnsubscriptionTask::NodeInfoUnsubscriptionTask(websocket::SessionId session_id,
                                                       websocket::QueryId query_id,
                                                       base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool NodeInfoUnsubscriptionTask::prepareArgs()
{
    // TODO
}


void NodeInfoUnsubscriptionTask::execute(lk::Core& core, SendResponse send_response)
{
    // TODO
}


AccountInfoUnsubscriptionTask::AccountInfoUnsubscriptionTask(websocket::SessionId session_id,
                                                             websocket::QueryId query_id,
                                                             base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool AccountInfoUnsubscriptionTask::prepareArgs()
{
    // TODO
}


void AccountInfoUnsubscriptionTask::execute(lk::Core& core, SendResponse send_response)
{
    // TODO
}

}


PublicService::PublicService(const base::PropertyTree& config, lk::Core& core)
  : _config{ config }
  , _core{ core }
  , _acceptor{ config, std::bind(&PublicService::createSession, this, std::placeholders::_1) }
{
    _core.subscribeToBlockAddition(std::bind(&PublicService::on_added_new_block, this, std::placeholders::_1));
    _core.subscribeToAnyTransactionStatusUpdate(std::bind(&PublicService::on_update_transaction_status, this, std::placeholders::_1));
    _core.subscribeToAnyAccountUpdate(std::bind(&PublicService::on_update_account, this, std::placeholders::_1));
}


PublicService::~PublicService()
{
    stop();
}


void PublicService::run()
{
    _acceptor.run();
    _worker = std::thread(&PublicService::task_worker, this);
}


void PublicService::stop()
{
    if (_worker.joinable()) {
        _worker.join();
    }
}


void PublicService::createSession(boost::asio::ip::tcp::socket&& socket)
{
    auto current_id = createId();
    auto session = std::make_unique<websocket::WebSocketSession>(
      std::move(socket),
      current_id,
      std::bind(&PublicService::on_session_request,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4),
      std::bind(&PublicService::on_session_close, this, std::placeholders::_1));
    _sessions_map.insert({ current_id, std::move(session) });
}


websocket::SessionId PublicService::createId()
{
    return ++_last_session_id;
}


void PublicService::on_session_request(websocket::SessionId session_id,
                                       websocket::QueryId query_id,
                                       websocket::Command::Id command_id,
                                       base::PropertyTree&& args)
{
    std::unique_ptr<tasks::Task> task;
    switch (command_id) {
        case websocket::Command::CALL_LAST_BLOCK_INFO:
            task = std::make_unique<tasks::NodeInfoCallTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::CALL_ACCOUNT_INFO:
            task = std::make_unique<tasks::AccountInfoCallTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::CALL_FIND_TRANSACTION_STATUS:
            task = std::make_unique<tasks::FindTransactionStatusTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::CALL_FIND_TRANSACTION:
            task = std::make_unique<tasks::FindTransactionTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::CALL_FIND_BLOCK:
            task = std::make_unique<tasks::FindBlockTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::SUBSCRIBE_PUSH_TRANSACTION:
            task = std::make_unique<tasks::PushTransactionSubscriptionTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::SUBSCRIBE_LAST_BLOCK_INFO:
            task = std::make_unique<tasks::NodeInfoSubscriptionTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::SUBSCRIBE_ACCOUNT_INFO:
            task = std::make_unique<tasks::AccountInfoSubscriptionTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::UNSUBSCRIBE_PUSH_TRANSACTION:
            task = std::make_unique<tasks::PushTransactionUnsubscriptionTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::UNSUBSCRIBE_LAST_BLOCK_INFO:
            task = std::make_unique<tasks::NodeInfoUnsubscriptionTask>(session_id, query_id, std::move(args));
            break;
        case websocket::Command::UNSUBSCRIBE_ACCOUNT_INFO:
            task = std::make_unique<tasks::AccountInfoUnsubscriptionTask>(session_id, query_id, std::move(args));
            break;
        default:
            // TODO fail
            return;
    }

    _tasks.push(std::move(task));
    LOG_INFO << "task pushed";
}


void PublicService::on_session_close(websocket::SessionId session_id)
{
    // TODO
}


[[noreturn]] void PublicService::task_worker() noexcept
{
    while (true) {
        try {
            LOG_INFO << "try task";
            _tasks.wait();
            auto task = _tasks.get();
            try {
                task->run(_core,
                          std::bind(&PublicService::on_send_response,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2,
                                    std::placeholders::_3));
            }
            catch (const base::Error& er) {
                LOG_DEBUG << er.what();
            }
            LOG_INFO << "executed task";
        }
        catch (...) {
            LOG_ERROR << "error at task execution";
        }
    }
}


void PublicService::on_send_response(websocket::SessionId session_id,
                                     websocket::QueryId query_id,
                                     base::PropertyTree&& result)
{
    auto sess = _sessions_map.find(session_id);
    ASSERT(sess != _sessions_map.end());
    sess->second->sendResult(query_id, std::move(result));
}


void PublicService::on_added_new_block(const lk::ImmutableBlock& block) {}

void PublicService::on_update_transaction_status(base::Sha256 tx_hash) {}

void PublicService::on_update_account(lk::Address accoutn_address) {}
