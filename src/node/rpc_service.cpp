#include "rpc_service.hpp"

#include "core/transaction.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"


RpcService::RpcService(const base::PropertyTree& config, lk::Core& core)
  : _config{ config }
  , _core{ core }
  , _server{ config }
{}


RpcService::~RpcService()
{
    stop();
}


void RpcService::run()
{
    _server.run([this](base::PropertyTree call, web_socket::ResponceCall response_callback) {
        if (response_callback) {
            return register_query(std::move(call), std::move(response_callback));
        }
        RAISE_ERROR(base::LogicError, "callback does not set");
    });
    _worker = std::thread(&RpcService::task_worker, this);
}


void RpcService::stop()
{
    if (_worker.joinable()) {
        _worker.join();
    }
}


void RpcService::register_query(base::PropertyTree call, web_socket::ResponceCall responce_callback)
{
    LOG_DEBUG << "received data" << call.toString();

    std::lock_guard lock(_tasks_mutex);
    _tasks.emplace_back(
      [this, query = std::move(call), callback = std::move(responce_callback)]() { callback(do_route(query)); });
}


base::PropertyTree RpcService::do_route(base::PropertyTree call)
{
    // do parsing and call specific method
}


[[noreturn]] void RpcService::task_worker()
{
    while (true) {
        std::optional<Task> current_task;
        {
            std::unique_lock lock(_tasks_mutex);
            _has_task.wait(lock, [this] { return !_tasks.empty(); });

            current_task = _tasks.front();
            _tasks.pop_front();
        }
        if (current_task) {
            current_task.value()();
        }
    }
}


lk::AccountInfo RpcService::getAccountInfo(const lk::Address& address)
{
    LOG_TRACE << "Received RPC request {getAccount}" << address;
    return _core.getAccountInfo(address);
}


web_socket::NodeInfo RpcService::getNodeInfo()
{
    LOG_TRACE << "Received RPC request {getNodeInfo}";
    auto& top_block = _core.getTopBlock();
    auto hash = base::Sha256::compute(base::toBytes(top_block));
    return { hash, top_block.getDepth() };
}


lk::Block RpcService::getBlock(const base::Sha256& block_hash)
{
    LOG_TRACE << "Received RPC request {getBlock} with block_hash[" << block_hash << "]";
    if (auto block_opt = _core.findBlock(block_hash); block_opt) {
        return *block_opt;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("Block was not found. hash[hex]:") + block_hash.toHex());
}


lk::Block RpcService::getBlock(uint64_t block_number)
{
    LOG_TRACE << "Received RPC request {getBlock} with block_number[" << block_number << "]";
    if (auto block_hash_opt = _core.findBlockHash(block_number); block_hash_opt) {
        if (auto block_opt = _core.findBlock(*block_hash_opt); block_opt) {
            return *block_opt;
        }
    }
    RAISE_ERROR(base::InvalidArgument, std::string("Block was not found. number:") + std::to_string(block_number));
}


lk::Transaction RpcService::getTransaction(const base::Sha256& transaction_hash)
{
    LOG_TRACE << "Received RPC request {getTransaction}";
    if (auto transaction_opt = _core.findTransaction(transaction_hash); transaction_opt) {
        return *transaction_opt;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("Transaction was not found. hash[hex]:") + transaction_hash.toHex());
}


lk::TransactionStatus RpcService::pushTransaction(const lk::Transaction& tx)
{
    LOG_TRACE << "Received RPC request {pushTransaction} with tx[" << tx << "]";
    return _core.addPendingTransaction(tx);
}


lk::TransactionStatus RpcService::getTransactionStatus(const base::Sha256& transaction_hash)
{
    LOG_TRACE << "Received RPC request {getTransactionStatus}";
    if (auto transaction_output_opt = _core.getTransactionOutput(transaction_hash); transaction_output_opt) {
        return *transaction_output_opt;
    }
    RAISE_ERROR(base::InvalidArgument,
                std::string("TransactionOutput was not found. hash[hex]:") + transaction_hash.toHex());
}


base::Bytes RpcService::callContractView(const lk::ViewCall& call)
{
    LOG_TRACE << "Received RPC request {callContractView}";
    return _core.callViewMethod(call);
}
