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
        auto hash = base::Sha256::compute(base::toBytes(_core.getTopBlock()));
        return { hash, base::config::RPC_PUBLIC_API_VERSION, 0 };
    }
    catch (const std::exception& e) {
        return { base::Sha256::null(), base::config::RPC_PUBLIC_API_VERSION, 0 };
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


rpc::TransactionStatus GeneralServerService::pushTransaction(const lk::Transaction& tx)
{
    LOG_TRACE << "Received RPC request {pushTransaction} with tx[" << tx << "]";

    try {
        _core.addPendingTransactionAndWait(tx);
        auto transaction_hash = base::Sha256::compute(base::toBytes(tx));
        const auto& [result_bytes, gas, status] = _core.getTransactionOutput(transaction_hash);
        switch (status) {
            case lk::TransactionStatus::SUCCESS:
                return rpc::TransactionStatus::createSuccess(gas, base::toHex(result_bytes));
            case lk::TransactionStatus::REVERT:
                return rpc::TransactionStatus::createRevert(gas, base::toHex(result_bytes));
            case lk::TransactionStatus::FAILED:
                return rpc::TransactionStatus::createFailed(gas, std::string{ "Evm result failed" });
        }
    }
    catch (const base::InvalidArgument& e) {
        return lk::TransactionStatus::createRejected(0, std::string{ "Error occurred: " } + e.what());
    }
    catch (const std::exception& e) {
        return lk::TransactionStatus::createFailed(0, std::string{ "Error occurred: " } + e.what());
    }
}


lk::TransactionStatus GeneralServerService::getTransactionResult(const base::Sha256& transaction_hash)
{
    LOG_TRACE << "Received RPC request {getTransactionResult}";
    try {
        return _core.getTransactionOutput(transaction_hash);
    }
    catch (const std::exception& e) {
        return { base::Sha256::null(), base::config::RPC_PUBLIC_API_VERSION, 0 };
    }
}


} // namespace node
