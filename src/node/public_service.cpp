#include "public_service.hpp"

#include "core/transaction.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"

namespace tasks
{

Task::Task(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args)
  : _session_id{ session_id }
  , _query_id{ query_id }
  , _args{ std::move(args) }
{}


void Task::run(PublicService& service)
{
    try {
        prepareArgs();
    }
    catch (const base::Error& error) {
        LOG_DEBUG << name() << "| prepare arguments for execution was failed:" << error.what();
        service.sendErrorResponse(_session_id, _query_id, error.what());
    }
    catch (...) {
        LOG_ERROR << name() << "| unexpected error during prepare arguments for execution";
    }

    try {
        execute(service);
    }
    catch (const base::Error& error) {
        LOG_DEBUG << name() << "execution was failed: ";
        service.sendErrorResponse(_session_id, _query_id, error.what());
    }
    catch (...) {
        LOG_ERROR << name() << "| unexpected error during execution";
    }
}


FindBlockTask::FindBlockTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void FindBlockTask::prepareArgs()
{
    if (_args.has_string_field("hash")) {
        _block_hash = websocket::deserializeHash(_args["hash"].as_string());
        return;
    }
    if (_args.has_number_field("depth")) {
        auto number_json_value = _args["depth"].as_number();
        if (!number_json_value.is_uint64()) {
            RAISE_ERROR(base::InvalidArgument, "args json \"depth\" member is not a uint type");
        }
        _block_depth = number_json_value.to_uint64();
        return;
    }
    RAISE_ERROR(base::InvalidArgument, "args json is not contain \"hash\" or \"depth\" member");
}


void FindBlockTask::execute(PublicService& service)
{
    if (_block_hash) {
        auto block = service._core.findBlock(_block_hash.value());
        if (block) {
            auto answer = websocket::serializeBlock(block.value());
            service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
        }
    }
    else if (_block_depth) {
        auto block_hash = service._core.findBlockHash(_block_depth.value());
        if (block_hash) {
            auto block = service._core.findBlock(block_hash.value());
            auto answer = websocket::serializeBlock(block.value());
            service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
        }
    }
}


const std::string& FindBlockTask::name() const noexcept
{
    static const std::string name("FindBlockTask");
    return name;
}


FindTransactionTask::FindTransactionTask(websocket::SessionId session_id,
                                         websocket::QueryId query_id,
                                         base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void FindTransactionTask::prepareArgs()
{
    if (!_args.has_string_field("hash")) {
        RAISE_ERROR(base::InvalidArgument, "args json is not contain a string \"hash\" member");
    }
    _tx_hash = websocket::deserializeHash(_args["hash"].as_string());
}


void FindTransactionTask::execute(PublicService& service)
{
    auto tx = service._core.findTransaction(_tx_hash.value());
    auto answer = websocket::serializeTransaction(tx.value());
    service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
}


const std::string& FindTransactionTask::name() const noexcept
{
    static const std::string name("FindTransactionTask");
    return name;
}


PushTransactionTask::PushTransactionTask(websocket::SessionId session_id,
                                         websocket::QueryId query_id,
                                         base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void PushTransactionTask::prepareArgs()
{
    _tx = websocket::deserializeTransaction(_args);
    _tx_hash = _tx->hashOfTransaction();
}


void PushTransactionTask::execute(PublicService& service)
{
    auto sess_registry_it = service._transaction_status_update_sessions_registry.find(_session_id);
    if (sess_registry_it != service._transaction_status_update_sessions_registry.end()) {
        if (sess_registry_it->second.contains(_tx_hash.value())) {
            return; // already exists callback on this operation
        }
    }

    auto sendResponse = std::bind(&PublicService::sendCorrectResponse,
                                  &service,
                                  std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3);
    auto sub_id = service._event_transaction_status_update.subscribe([session_id = this->_session_id,
                                                                      query_id = this->_query_id,
                                                                      sendResponse = std::move(sendResponse),
                                                                      tx_hash = _tx_hash.value(),
                                                                      &core = service._core](base::Sha256 updated_tx) {
        if (updated_tx == tx_hash) {
            auto tx_status = core.getTransactionOutput(updated_tx);
            ASSERT(tx_status);
            auto answer = websocket::serializeTransactionStatus(tx_status.value());
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


const std::string& PushTransactionTask::name() const noexcept
{
    static const std::string name("PushTransactionTask");
    return name;
}


FindTransactionStatusTask::FindTransactionStatusTask(websocket::SessionId session_id,
                                                     websocket::QueryId query_id,
                                                     base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void FindTransactionStatusTask::prepareArgs()
{
    if (!_args.has_string_field("hash")) {
        RAISE_ERROR(base::InvalidArgument, "args json is not contain a string \"hash\" member");
    }
    _tx_hash = websocket::deserializeHash(_args["hash"].as_string());
}


void FindTransactionStatusTask::execute(PublicService& service)
{
    auto tx_status = service._core.getTransactionOutput(_tx_hash.value());
    if (tx_status) {
        auto answer = websocket::serializeTransactionStatus(tx_status.value());
        service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
        return;
    }
    LOG_DEBUG << "Cant find transaction status task";
}


const std::string& FindTransactionStatusTask::name() const noexcept
{
    static const std::string name("FindTransactionStatusTask");
    return name;
}


NodeInfoCallTask::NodeInfoCallTask(websocket::SessionId session_id,
                                   websocket::QueryId query_id,
                                   base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void NodeInfoCallTask::prepareArgs()
{
    // Do nothing
}


void NodeInfoCallTask::execute(PublicService& service)
{
    auto last_block_hash = service._core.getTopBlockHash();
    auto last_block_number = service._core.getTopBlock().getDepth();
    websocket::NodeInfo info{ last_block_hash, last_block_number };
    auto answer = websocket::serializeInfo(info);
    service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
}


const std::string& NodeInfoCallTask::name() const noexcept
{
    static const std::string name("NodeInfoCallTask");
    return name;
}


NodeInfoSubscribeTask::NodeInfoSubscribeTask(websocket::SessionId session_id,
                                             websocket::QueryId query_id,
                                             base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void NodeInfoSubscribeTask::prepareArgs()
{
    // Do nothing
}


void NodeInfoSubscribeTask::execute(PublicService& service)
{
    auto iter = service._info_update_sessions_registry.find(_session_id);
    if (iter != service._info_update_sessions_registry.end()) {
        return;
    }

    auto sendResponse = std::bind(&PublicService::sendCorrectResponse,
                                  &service,
                                  std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3);
    auto sub_id = service._event_block_added.subscribe(
      [session_id = this->_session_id, query_id = this->_query_id, sendResponse = std::move(sendResponse)](
        lk::ImmutableBlock block) {
          websocket::NodeInfo info{ block.getHash(), block.getDepth() };
          auto answer = websocket::serializeInfo(info);
          sendResponse(session_id, query_id, std::move(answer));
      });

    service._info_update_sessions_registry.insert({ _session_id, sub_id });
}


const std::string& NodeInfoSubscribeTask::name() const noexcept
{
    static const std::string name("NodeInfoSubscribeTask");
    return name;
}


NodeInfoUnsubscribeTask::NodeInfoUnsubscribeTask(websocket::SessionId session_id,
                                                 websocket::QueryId query_id,
                                                 base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void NodeInfoUnsubscribeTask::prepareArgs()
{
    // Do nothing
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


const std::string& NodeInfoUnsubscribeTask::name() const noexcept
{
    static const std::string name("NodeInfoUnsubscribeTask");
    return name;
}


AccountInfoCallTask::AccountInfoCallTask(websocket::SessionId session_id,
                                         websocket::QueryId query_id,
                                         base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void AccountInfoCallTask::prepareArgs()
{
    if (!_args.has_string_field("address")) {
        RAISE_ERROR(base::InvalidArgument, "args json is not contain a string\"address\" member");
    }
    _address = websocket::deserializeAddress(_args["address"].as_string());
}


void AccountInfoCallTask::execute(PublicService& service)
{
    auto account_info = service._core.getAccountInfo(_address.value());
    auto answer = websocket::serializeAccountInfo(account_info);
    service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
}


const std::string& AccountInfoCallTask::name() const noexcept
{
    static const std::string name("AccountInfoCallTask");
    return name;
}


FeeInfoCallTask::FeeInfoCallTask(websocket::SessionId session_id,
                                         websocket::QueryId query_id,
                                         base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void FeeInfoCallTask::prepareArgs()
{
}


void FeeInfoCallTask::execute(PublicService& service)
{
    auto last_block_hash = service._core.getTopBlockHash();
    auto block = service._core.findBlock(last_block_hash);
    //TODO: if(block) false
    //answer["info"] = websocket::serializeMidFee(block.value());
    auto answer = base::json::Value();
    answer["fee"] = websocket::serializeMidFee(block.value());
    service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
}


const std::string& FeeInfoCallTask::name() const noexcept
{
    static const std::string name("FeeInfoCallTask");
    return name;
}


AccountInfoSubscribeTask::AccountInfoSubscribeTask(websocket::SessionId session_id,
                                                   websocket::QueryId query_id,
                                                   base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void AccountInfoSubscribeTask::prepareArgs()
{
    if (!_args.has_string_field("address")) {
        RAISE_ERROR(base::InvalidArgument, "args json is not contain a string\"address\" member");
    }
    _address = websocket::deserializeAddress(_args["address"].as_string());
}


void AccountInfoSubscribeTask::execute(PublicService& service)
{

    auto sess_registry_it = service._account_update_sessions_registry.find(_session_id);
    if (sess_registry_it != service._account_update_sessions_registry.end()) {
        if (sess_registry_it->second.contains(_address.value())) {
            return; // already exists callback on this operation
        }
    }

    auto sendResponse = std::bind(&PublicService::sendCorrectResponse,
                                  &service,
                                  std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3);
    auto sub_id = service._event_account_update.subscribe([session_id = this->_session_id,
                                                           query_id = this->_query_id,
                                                           sendResponse = std::move(sendResponse),
                                                           address = _address.value(),
                                                           &core = service._core](lk::Address updated_address) {
        if (updated_address == address) {
            auto account_info = core.getAccountInfo(updated_address);
            auto answer = websocket::serializeAccountInfo(account_info);
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


const std::string& AccountInfoSubscribeTask::name() const noexcept
{
    static const std::string name("AccountInfoSubscribeTask");
    return name;
}


AccountInfoUnsubscribeTask::AccountInfoUnsubscribeTask(websocket::SessionId session_id,
                                                       websocket::QueryId query_id,
                                                       base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void AccountInfoUnsubscribeTask::prepareArgs()
{
    if (!_args.has_string_field("address")) {
        RAISE_ERROR(base::InvalidArgument, "args json is not contain a string\"address\" member");
    }
    _address = websocket::deserializeAddress(_args["address"].as_string());
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


const std::string& AccountInfoUnsubscribeTask::name() const noexcept
{
    static const std::string name("AccountInfoUnsubscribeTask");
    return name;
}


UnsubscribeTransactionStatusUpdateTask::UnsubscribeTransactionStatusUpdateTask(websocket::SessionId session_id,
                                                                               websocket::QueryId query_id,
                                                                               base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void UnsubscribeTransactionStatusUpdateTask::prepareArgs()
{
    if (!_args.has_string_field("hash")) {
        RAISE_ERROR(base::InvalidArgument, "args json is not contain a string \"hash\" member");
    }
    _tx_hash = websocket::deserializeHash(_args["hash"].as_string());
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


const std::string& UnsubscribeTransactionStatusUpdateTask::name() const noexcept
{
    static const std::string name("UnsubscribeTransactionStatusUpdateTask");
    return name;
}


LoginTask::LoginTask(websocket::SessionId session_id, websocket::QueryId query_id, base::json::Value&& args)
  : Task{ session_id, query_id, std::move(args) }
{}


void LoginTask::prepareArgs()
{
    if (!_args.has_string_field("login")) {
        RAISE_ERROR(base::InvalidArgument, "args json is not contain a string \"login\" member");
    }
    _login = websocket::deserializeLogin(_args["login"].as_string());
}


void LoginTask::execute(PublicService& service)
{
    if (_login == service._login) {
        service.addAdminSession(_session_id);
        auto answer = websocket::serializeSessionId(_session_id);
        service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
    }
    else {
        auto answer = websocket::serializeMessage("Incorrect login");
        service.sendCorrectResponse(_session_id, _query_id, std::move(answer));
    }
}


const std::string& LoginTask::name() const noexcept
{
    static const std::string name("LoginTask");
    return name;
}

}


PublicService::PublicService(base::json::Value config, lk::Core& core)
  : _core{ core }
  , _login{ config["login"].as_string() }
  , _acceptor{ std::move(config), std::bind(&PublicService::createSession, this, std::placeholders::_1) }

{
    _core.subscribeToBlockAddition(std::bind(&PublicService::on_added_new_block, this, std::placeholders::_1));
    _core.subscribeToAnyTransactionStatusUpdate(
      std::bind(&PublicService::on_updated_transaction_status, this, std::placeholders::_1));
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
                                       base::json::Value&& args)
{
    switch (command_id) {
        case websocket::Command::CALL_LAST_BLOCK_INFO:
            _input_tasks.push(std::make_unique<tasks::NodeInfoCallTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::CALL_ACCOUNT_INFO:
            _input_tasks.push(std::make_unique<tasks::AccountInfoCallTask>(session_id, query_id, std::move(args)));
            break;
        case websocket::Command::CALL_FEE_INFO:
            _input_tasks.push(std::make_unique<tasks::FeeInfoCallTask>(session_id, query_id, std::move(args)));
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
        case websocket::Command::LOGIN:
            _input_tasks.push(std::make_unique<tasks::LoginTask>(session_id, query_id, std::move(args)));
            break;
        default:
            // TODO fail
            break;
    }
}


void PublicService::on_session_close(websocket::SessionId session_id)
{
    auto closed_admin_session = std::find(_admins_sessions.begin(), _admins_sessions.end(), session_id);
    if (closed_admin_session != _admins_sessions.end()) {
        _admins_sessions.erase(closed_admin_session);
    }
    // TODO
}


[[noreturn]] void PublicService::task_worker() noexcept
{
    while (true) {
        LOG_DEBUG << "wait for a task";
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


void PublicService::sendCorrectResponse(websocket::SessionId session_id,
                                        websocket::QueryId query_id,
                                        base::json::Value&& result)
{
    auto sess = _running_sessions.find(session_id);
    ASSERT(sess != _running_sessions.end());
    sess->second->sendResult(query_id, std::move(result));
}


void PublicService::sendErrorResponse(websocket::SessionId session_id,
                                      websocket::QueryId query_id,
                                      const std::string& error_message)
{
    auto sess = _running_sessions.find(session_id);
    ASSERT(sess != _running_sessions.end());
    sess->second->sendErrorMessage(query_id, error_message);
}


void PublicService::on_added_new_block(const lk::ImmutableBlock& block)
{
    _event_block_added.notify(block);
}


void PublicService::on_updated_transaction_status(base::Sha256 tx_hash)
{
    _event_transaction_status_update.notify(tx_hash);
}


void PublicService::on_update_account(lk::Address account_address)
{
    _event_account_update.notify(account_address);
}

void PublicService::addAdminSession(const websocket::SessionId& id)
{
    if (std::find(_admins_sessions.begin(), _admins_sessions.end(), id) == _admins_sessions.end()) {
        _admins_sessions.push_back(id);
    }
}
