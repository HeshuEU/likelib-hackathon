#include "rpc_service.hpp"

#include "core/transaction.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"

namespace node
{

GeneralServerService::GeneralServerService(lk::Core& core)
  : _core{ core }
{}


lk::AccountInfo GeneralServerService::getAccountInfo(const lk::Address& address)
{
    LOG_TRACE << "Received RPC request {getAccount}" << address;
    return _core.getAccountInfo(address);
}


rpc::Info GeneralServerService::getNodeInfo()
{
    LOG_TRACE << "Received RPC request {getNodeInfo}";
    const auto& top_block = _core.getTopBlock();
    auto hash = base::Sha256::compute(base::toBytes(top_block));
    return { hash, top_block.getDepth(), base::config::RPC_PUBLIC_API_VERSION };
}


lk::ImmutableBlock GeneralServerService::getBlock(const base::Sha256& block_hash)
{
    LOG_TRACE << "Received RPC request {getBlock} with block_hash[" << block_hash << "]";
    if (auto block_opt = _core.findBlock(block_hash); block_opt) {
        return *block_opt;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("Block was not found. hash[hex]:") + block_hash.toHex());
}


lk::ImmutableBlock GeneralServerService::getBlock(uint64_t block_number)
{
    LOG_TRACE << "Received RPC request {getBlock} with block_number[" << block_number << "]";
    if (auto block_hash_opt = _core.findBlockHash(block_number); block_hash_opt) {
        if (auto block_opt = _core.findBlock(*block_hash_opt); block_opt) {
            return *block_opt;
        }
    }
    RAISE_ERROR(base::InvalidArgument, std::string("Block was not found. number:") + std::to_string(block_number));
}


lk::Transaction GeneralServerService::getTransaction(const base::Sha256& transaction_hash)
{
    LOG_TRACE << "Received RPC request {getTransaction}";
    if (auto transaction_opt = _core.findTransaction(transaction_hash); transaction_opt) {
        return *transaction_opt;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("Transaction was not found. hash[hex]:") + transaction_hash.toHex());
}


lk::TransactionStatus GeneralServerService::pushTransaction(const lk::Transaction& tx)
{
    LOG_TRACE << "Received RPC request {pushTransaction} with tx[" << tx << "]";
    return _core.addPendingTransaction(tx);
}


lk::TransactionStatus GeneralServerService::getTransactionStatus(const base::Sha256& transaction_hash)
{
    LOG_TRACE << "Received RPC request {getTransactionStatus}";
    if (auto transaction_output_opt = _core.getTransactionOutput(transaction_hash); transaction_output_opt) {
        return *transaction_output_opt;
    }
    RAISE_ERROR(base::InvalidArgument,
                std::string("TransactionOutput was not found. hash[hex]:") + transaction_hash.toHex());
}


} // namespace node
