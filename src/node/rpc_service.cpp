#include "rpc_service.hpp"

#include <core/transaction.hpp>

#include <base/config.hpp>
#include <base/hash.hpp>
#include <base/log.hpp>

namespace node
{

GeneralServerService::GeneralServerService(lk::Core& core)
  : _core{ core }
{}


lk::AccountInfo GeneralServerService::getAccount(const lk::Address& address)
{
    LOG_TRACE << "Received RPC request {getAccount}" << address;
    return _core.getAccountInfo(address);
}


rpc::Info GeneralServerService::getNodeInfo()
{
    LOG_TRACE << "Received RPC request {getNodeInfo}";
    try {
        auto& top_block = _core.getTopBlock();
        auto hash = base::Sha256::compute(base::toBytes(top_block));
        return { hash, top_block.getDepth(), base::config::RPC_PUBLIC_API_VERSION, 0 };
    }
    catch (const std::exception& e) {
        return { base::Sha256::null(), 0, base::config::RPC_PUBLIC_API_VERSION, 0 };
    }
}


lk::Block GeneralServerService::getBlock(const base::Sha256& block_hash)
{
    LOG_TRACE << "Received RPC request {getBlock} with block_hash[" << block_hash << "]";
    if (auto block_opt = _core.findBlock(block_hash); block_opt) {
        return *block_opt;
    }
    else {
        return lk::invalidBlock();
    }
}


lk::Block GeneralServerService::getBlock(uint64_t block_number)
{
    LOG_TRACE << "Received RPC request {getBlock} with block_number[" << block_number << "]";

    if (auto block_opt = _core.findBlockHash(block_number); block_opt) {
        return _core.findBlock(block_opt.value()).value();
    }
    else {
        return lk::invalidBlock();
    }
}


lk::Transaction GeneralServerService::getTransaction(const base::Sha256& transaction_hash)
{
    LOG_TRACE << "Received RPC request {getTransaction}";
    if (auto transaction_opt = _core.findTransaction(transaction_hash); transaction_opt) {
        return *transaction_opt;
    }
    else {
        return lk::invalidTransaction();
    }
}


lk::TransactionStatus GeneralServerService::pushTransaction(const lk::Transaction& tx)
{
    LOG_TRACE << "Received RPC request {pushTransaction} with tx[" << tx << "]";

    try {
        _core.addPendingTransactionAndWait(tx);
        auto transaction_hash = tx.hashOfTransaction();
        auto status = _core.getTransactionOutput(transaction_hash);
        return status;
    }
    catch (const std::exception& e) {
        return lk::TransactionStatus(lk::TransactionStatus::StatusCode::Failed,
                                     lk::TransactionStatus::ActionType::None,
                                     0,
                                     std::string{ "Error occurred: " } + e.what());
    }
}


lk::TransactionStatus GeneralServerService::getTransactionResult(const base::Sha256& transaction_hash)
{
    LOG_TRACE << "Received RPC request {getTransactionResult}";
    try {
        return _core.getTransactionOutput(transaction_hash);
    }
    catch (const std::exception& e) {
        return lk::TransactionStatus(lk::TransactionStatus::StatusCode::Failed,
                                     lk::TransactionStatus::ActionType::None,
                                     0,
                                     std::string{ "Error occurred: " } + e.what());
    }
}


base::Bytes GeneralServerService::callContractView(const lk::Address& from,
                                                   const lk::Address& contract_address,
                                                   const base::Bytes& message)
{
    LOG_TRACE << "Received RPC request {callContractView}";
    try {
        return _core.callViewMethod(from, contract_address, message);
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::Error, e.what());
    }
}

} // namespace node
