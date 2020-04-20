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


lk::AccountInfo GeneralServerService::get_account(const lk::Address& address)
{
    LOG_TRACE << "Received RPC request {get_account}" << address;
    return _core.getAccountInfo(address);
}


rpc::Info GeneralServerService::get_node_info()
{
    LOG_TRACE << "Received RPC request {get_node_info}";
    try {
        auto hash = base::Sha256::compute(base::toBytes(_core.getTopBlock()));
        return { hash, base::config::RPC_PUBLIC_API_VERSION, 0 };
    }
    catch (const std::exception& e) {
        return { base::Sha256::null(), base::config::RPC_PUBLIC_API_VERSION, 0 };
    }
}


lk::Block GeneralServerService::get_block(const base::Sha256& block_hash)
{
    LOG_TRACE << "Received RPC request {get_block} with block_hash[" << block_hash << "]";
    if (auto block_opt = _core.findBlock(block_hash); block_opt) {
        return *block_opt;
    }
    else {
        return lk::invalidBlock();
    }
}


lk::Transaction GeneralServerService::get_transaction(const base::Sha256& transaction_hash)
{
    LOG_TRACE << "Received RPC request {get_transaction}";
    if (auto transaction_opt = _core.findTransaction(transaction_hash); transaction_opt) {
        return *transaction_opt;
    }
    else {
        return lk::invalidTransaction();
    }
}


rpc::TransactionStatus GeneralServerService::push_transaction(lk::Transaction tx)
{
    LOG_TRACE << "Received RPC request {push_transaction} with tx[" << tx << "]";

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
        return rpc::TransactionStatus::createRejected(0, std::string{ "Error occurred: " } + e.what());
    }
    catch (const std::exception& e) {
        return rpc::TransactionStatus::createFailed(0, std::string{ "Error occurred: " } + e.what());
    }
}


} // namespace node
