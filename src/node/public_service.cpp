#include "public_service.hpp"

#include "core/transaction.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"

namespace tasks
{

Task::Task(websocket::SessionId session_id, websocket::QueryId query_id, base::PropertyTree&& args)
  : _session_id{ session_id }
  , _query_id{ query_id }
  , _args{ std::move(args) }
{}


void Task::run(PublicService& service)
{
    if (prepareArgs()) {
        return execute(service);
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


void FindBlockTask::execute(PublicService& service)
{
    if (_block_hash) {
        auto block = service._core.findBlock(_block_hash.value());
        if (block) {
            base::PropertyTree answer = websocket::serializeBlock(block.value());
            service.sendResponse(_session_id, _query_id, std::move(answer));
        }
    }
    else {
        auto block_hash = service._core.findBlockHash(_block_number.value());
        if (block_hash) {
            auto block = service._core.findBlock(block_hash.value());
            base::PropertyTree answer = websocket::serializeBlock(block.value());
            service.sendResponse(_session_id, _query_id, std::move(answer));
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


void FindTransactionTask::execute(PublicService& service)
{
    auto tx = service._core.findTransaction(_tx_hash.value());
    base::PropertyTree answer = websocket::serializeTransaction(tx.value());
    service.sendResponse(_session_id, _query_id, std::move(answer));
}


PushTransactionTask::PushTransactionTask(websocket::SessionId session_id,
                                         websocket::QueryId query_id,
                                         base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool PushTransactionTask::prepareArgs()
{
    _tx = websocket::deserializeTransaction(_args);
    if (!_tx) {
        LOG_DEBUG << "deserialization error";
        return false;
    }
    _tx_hash = _tx->hashOfTransaction();

    return true;
}


void PushTransactionTask::execute(PublicService& service)
{
    auto sess_registry_it = service._transaction_status_update_sessions_registry.find(_session_id);
    if (sess_registry_it != service._transaction_status_update_sessions_registry.end()) {
        if (sess_registry_it->second.contains(_tx_hash.value())) {
            return; // already exists callback on this operation
        }
    }

    auto sendResponse = std::bind(
      &PublicService::sendResponse, &service, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto sub_id = service._event_transaction_status_update.subscribe([session_id = this->_session_id,
                                                                      query_id = this->_query_id,
                                                                      sendResponse = std::move(sendResponse),
                                                                      tx_hash = _tx_hash.value(),
                                                                      &core = service._core](base::Sha256 updated_tx) {
        if (updated_tx == tx_hash) {
            auto tx_status = core.getTransactionOutput(updated_tx);
            ASSERT(tx_status);
            base::PropertyTree answer = websocket::serializeTransactionStatus(tx_status.value());
            sendResponse(session_id, query_id, std::move(answer));
        }
    });

    if (sess_registry_it == service._transaction_status_update_sessions_registry.end()) {
        service._transaction_status_update_sessions_registry.insert({ _session_id, {} })
          .first->second.insert({ _tx_hash.value(), sub_id });
    }
    else {
        sess_registry_it->second.insert({ _tx_hash.value(), sub_id });
    }

    service._core.addPendingTransaction(_tx.value());
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


void FindTransactionStatusTask::execute(PublicService& service)
{
    auto tx_status = service._core.getTransactionOutput(_tx_hash.value());
    if (tx_status) {
        base::PropertyTree answer = websocket::serializeTransactionStatus(tx_status.value());
        service.sendResponse(_session_id, _query_id, std::move(answer));
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


void NodeInfoCallTask::execute(PublicService& service)
{
    auto last_block_hash = service._core.getTopBlockHash();
    auto last_block_number = service._core.getTopBlock().getDepth();
    websocket::NodeInfo info{ last_block_hash, last_block_number };
    base::PropertyTree answer = websocket::serializeInfo(info);
    service.sendResponse(_session_id, _query_id, std::move(answer));
}


NodeInfoSubscribeTask::NodeInfoSubscribeTask(websocket::SessionId session_id,
                                             websocket::QueryId query_id,
                                             base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool NodeInfoSubscribeTask::prepareArgs()
{
    return true;
}


void NodeInfoSubscribeTask::execute(PublicService& service)
{
    auto iter = service._info_update_sessions_registry.find(_session_id);
    if (iter != service._info_update_sessions_registry.end()) {
        return;
    }

    auto sendResponse = std::bind(
      &PublicService::sendResponse, &service, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto sub_id = service._event_block_added.subscribe(
      [session_id = this->_session_id, query_id = this->_query_id, sendResponse = std::move(sendResponse)](
        lk::ImmutableBlock block) {
          websocket::NodeInfo info{ block.getHash(), block.getDepth() };
          base::PropertyTree answer = websocket::serializeInfo(info);
          sendResponse(session_id, query_id, std::move(answer));
      });

    service._info_update_sessions_registry.insert({ _session_id, sub_id });
}


NodeInfoUnsubscribeTask::NodeInfoUnsubscribeTask(websocket::SessionId session_id,
                                                 websocket::QueryId query_id,
                                                 base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool NodeInfoUnsubscribeTask::prepareArgs()
{
    return true;
}


void NodeInfoUnsubscribeTask::execute(PublicService& service)
{
    auto iter = service._info_update_sessions_registry.find(_session_id);
    if (iter == service._info_update_sessions_registry.end()) {
        return;
    }

    service._event_block_added.unsubscribe(iter->second);
    service._info_update_sessions_registry.erase(iter);
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


void AccountInfoCallTask::execute(PublicService& service)
{
    auto account_info = service._core.getAccountInfo(_address.value());
    base::PropertyTree answer = websocket::serializeAccountInfo(account_info);
    service.sendResponse(_session_id, _query_id, std::move(answer));
}


AccountInfoSubscribeTask::AccountInfoSubscribeTask(websocket::SessionId session_id,
                                                   websocket::QueryId query_id,
                                                   base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool AccountInfoSubscribeTask::prepareArgs()
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


void AccountInfoSubscribeTask::execute(PublicService& service)
{

    auto sess_registry_it = service._account_update_sessions_registry.find(_session_id);
    if (sess_registry_it != service._account_update_sessions_registry.end()) {
        if (sess_registry_it->second.contains(_address.value())) {
            return; // already exists callback on this operation
        }
    }

    auto sendResponse = std::bind(
      &PublicService::sendResponse, &service, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto sub_id = service._event_account_update.subscribe([session_id = this->_session_id,
                                                           query_id = this->_query_id,
                                                           sendResponse = std::move(sendResponse),
                                                           address = _address.value(),
                                                           &core = service._core](lk::Address updated_address) {
        if (updated_address == address) {
            auto account_info = core.getAccountInfo(updated_address);
            base::PropertyTree answer = websocket::serializeAccountInfo(account_info);
            sendResponse(session_id, query_id, std::move(answer));
        }
    });

    if (sess_registry_it == service._account_update_sessions_registry.end()) {
        std::unordered_map<lk::Address, std::size_t> temp{};
        service._account_update_sessions_registry.insert({ _session_id, {} })
          .first->second.insert({ _address.value(), sub_id });
    }
    else {
        sess_registry_it->second.insert({ _address.value(), sub_id });
    }
}


AccountInfoUnsubscribeTask::AccountInfoUnsubscribeTask(websocket::SessionId session_id,
                                                       websocket::QueryId query_id,
                                                       base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool AccountInfoUnsubscribeTask::prepareArgs()
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


void AccountInfoUnsubscribeTask::execute(PublicService& service)
{
    auto sess_registry_it = service._account_update_sessions_registry.find(_session_id);
    if (sess_registry_it == service._account_update_sessions_registry.end()) {
        return;
    }

    auto target_action = sess_registry_it->second.find(_address.value());
    if (target_action == sess_registry_it->second.end()) {
        return;
    }

    service._event_account_update.unsubscribe(target_action->second);
    sess_registry_it->second.erase(target_action);
}


UnsubscribeTransactionStatusUpdateTask::UnsubscribeTransactionStatusUpdateTask(websocket::SessionId session_id,
                                                                               websocket::QueryId query_id,
                                                                               base::PropertyTree&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


bool UnsubscribeTransactionStatusUpdateTask::prepareArgs()
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


void UnsubscribeTransactionStatusUpdateTask::execute(PublicService& service)
{
    auto sess_registry_it = service._transaction_status_update_sessions_registry.find(_session_id);
    if (sess_registry_it == service._transaction_status_update_sessions_registry.end()) {
        return;
    }

    auto target_action = sess_registry_it->second.find(_tx_hash.value());
    if (target_action == sess_registry_it->second.end()) {
        return;
    }

    service._event_transaction_status_update.unsubscribe(target_action->second);
    sess_registry_it->second.erase(target_action);
}

}

PublicService::PublicService(const base::PropertyTree& config, lk::Core& core)
  : _config{ config }
  , _core{ core }
  , _acceptor{ config, std::bind(&PublicService::createSession, this, std::placeholders::_1) }
{
    _core.subscribeToBlockAddition(std::bind(&PublicService::on_added_new_block, this, std::placeholders::_1));
    _core.subscribeToAnyTransactionStatusUpdate(
      std::bind(&PublicService::on_update_transaction_status, this, std::placeholders::_1));
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
    _running_sessions.insert({ current_id, std::move(session) });
}


websocket::SessionId PublicService::createId()
{
    return ++_last_given_session_id;
}


void PublicService::on_session_request(websocket::SessionId session_id,
                                       websocket::QueryId query_id,
                                       websocket::Command::Id command_id,
                                       base::PropertyTree&& args)
{
    switch (command_id) {
        case websocket::Command::CALL_LAST_BLOCK_INFO:
            _input_tasks.push(std::make_unique<tasks::NodeInfoCallTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::CALL_ACCOUNT_INFO:
            _input_tasks.push(std::make_unique<tasks::AccountInfoCallTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::CALL_FIND_TRANSACTION_STATUS:
            _input_tasks.push(
              std::make_unique<tasks::FindTransactionStatusTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::CALL_FIND_TRANSACTION:
            _input_tasks.push(std::make_unique<tasks::FindTransactionTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::CALL_FIND_BLOCK:
            _input_tasks.push(std::make_unique<tasks::FindBlockTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::SUBSCRIBE_PUSH_TRANSACTION:
            _input_tasks.push(std::make_unique<tasks::PushTransactionTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::SUBSCRIBE_LAST_BLOCK_INFO:
            _input_tasks.push(std::make_unique<tasks::NodeInfoSubscribeTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::SUBSCRIBE_ACCOUNT_INFO:
            _input_tasks.push(std::make_unique<tasks::AccountInfoSubscribeTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::UNSUBSCRIBE_PUSH_TRANSACTION:
            _input_tasks.push(
              std::make_unique<tasks::UnsubscribeTransactionStatusUpdateTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::UNSUBSCRIBE_LAST_BLOCK_INFO:
            _input_tasks.push(std::make_unique<tasks::NodeInfoUnsubscribeTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::UNSUBSCRIBE_ACCOUNT_INFO:
            break;
        default:
            // TODO fail
            break;
    }
}


void PublicService::on_session_close(websocket::SessionId session_id)
{
    // TODO
}


[[noreturn]] void PublicService::task_worker() noexcept
{
    while (true) {
        LOG_DEBUG << "wait for a task";
        _input_tasks.wait();
        auto task = _input_tasks.get();
        try {
            task->run(*this);
        }
        catch (const base::Error& er) {
            LOG_DEBUG << er.what();
        }
        catch (...) {
            LOG_ERROR << "error at task execution";
        }
        LOG_DEBUG << "task executed";
    }
}


void PublicService::sendResponse(websocket::SessionId session_id,
                                 websocket::QueryId query_id,
                                 base::PropertyTree&& result)
{
    auto sess = _running_sessions.find(session_id);
    ASSERT(sess != _running_sessions.end());
    sess->second->sendResult(query_id, std::move(result));
}


void PublicService::on_added_new_block(const lk::ImmutableBlock& block)
{
    _event_block_added.notify(block);
}


void PublicService::on_update_transaction_status(base::Sha256 tx_hash)
{
    _event_transaction_status_update.notify(tx_hash);
}


void PublicService::on_update_account(lk::Address account_address)
{
    _event_account_update.notify(account_address);
}
