#include "rpc_service.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"
#include "core/transaction.hpp"

namespace node
{

GeneralServerService::GeneralServerService(lk::Core& core)
  : _core{ core }
{
    LOG_TRACE << "Constructed GeneralServerService";
}


GeneralServerService::~GeneralServerService()
{
    LOG_TRACE << "Destructed GeneralServerService";
}


rpc::OperationStatus GeneralServerService::test(uint32_t api_version)
{
    LOG_TRACE << "Received RPC request {test} with api_version[" << api_version << "]";
    if (base::config::RPC_PUBLIC_API_VERSION == api_version) {
        return rpc::OperationStatus::createSuccess("RPC api is compatible");
    }
    else {
        if (base::config::RPC_PUBLIC_API_VERSION > api_version) {
            return rpc::OperationStatus::createFailed("Your RPC api is old.");
        }
        return rpc::OperationStatus::createFailed("Not support api version");
    }
}


lk::Balance GeneralServerService::balance(const lk::Address& address)
{
    try {
        LOG_TRACE << "Received RPC request {balance} with address[" << address.toString() << "]";
        auto ret = _core.getBalance(address);
        return ret;
    }
    catch (const std::exception& e) {
        LOG_WARNING << "Exception caught during balance request: " << e.what();
        return -1;
    }
    catch (...) {
        LOG_WARNING << "Exception caught during balance request: unknown";
        return -1;
    }
}


rpc::Info GeneralServerService::info()
{
    LOG_TRACE << "Received RPC request {info}";
    try {
        auto hash = base::Sha256::compute(base::toBytes(_core.getTopBlock()));
        return { hash, 0 };
    }
    catch (const std::exception& e) {
        return { base::Sha256::null(), 0 };
    }
}


lk::Block GeneralServerService::get_block(const base::Sha256& block_hash)
{
    LOG_TRACE << "Received RPC request {get_block} with block_hash[" << block_hash << "]";
    if (auto block_opt = _core.findBlock(block_hash); block_opt) {
        return *block_opt;
    }
    else {
        return lk::Block{ lk::BlockDepth(-1), base::Sha256::null(), base::Time(0), lk::Address::null(), {} };
    }
}


std::tuple<rpc::OperationStatus, lk::Address, lk::Balance> GeneralServerService::transaction_create_contract(
  lk::Balance amount,
  const lk::Address& from_address,
  const base::Time& timestamp,
  lk::Balance gas,
  const std::string& hex_contract_code,
  const std::string& hex_init,
  const lk::Sign& signature)
{
    LOG_TRACE << "Received RPC request {transaction_to_contract} with from_address[" << from_address.toString()
              << "], amount[" << amount << "], gas" << gas << "], code[" << hex_contract_code << "], timestamp["
              << timestamp.getSecondsSinceEpoch() << "], initial_message[" << hex_init << "]";

    auto contract_code = base::fromHex<base::Bytes>(hex_contract_code);
    auto init = base::fromHex<base::Bytes>(hex_init);

    lk::TransactionBuilder txb;
    txb.setType(lk::Transaction::Type::CONTRACT_CREATION);
    txb.setFrom(from_address);
    txb.setTo(lk::Address::null());
    txb.setAmount(amount);
    txb.setFee(gas);
    txb.setTimestamp(timestamp);

    lk::ContractInitData data(std::move(contract_code), std::move(init));
    txb.setData(base::toBytes(data));

    txb.setSign(signature);

    auto tx = std::move(txb).build();

    LOG_DEBUG << tx;

    try {
        _core.addPendingTransactionAndWait(tx);
        auto hash = base::Sha256::compute(base::toBytes(tx));
        auto raw_output = _core.getTransactionOutput(hash);
        if (raw_output.isEmpty()) {
            return { rpc::OperationStatus::createFailed(std::string{ "Transaction failed" }),
                     lk::Address::null(),
                     gas };
        }
        base::SerializationIArchive ia(raw_output);
        auto is_successful = ia.deserialize<bool>();
        if (!is_successful) {
            return { rpc::OperationStatus::createFailed(std::string{ "Transaction failed" }),
                     lk::Address::null(),
                     gas };
        }
        auto contract_address = ia.deserialize<lk::Address>();
        auto output = ia.deserialize<base::Bytes>();
        auto gas_left = ia.deserialize<lk::Balance>();

        std::string ret_str{ "Contract was successfully deployed" };
        if (!output.isEmpty()) {
            ret_str += " with output: ";
            ret_str += base::toHex<base::Bytes>(output);
        }
        return { rpc::OperationStatus::createSuccess(ret_str), contract_address, gas_left };
    }
    catch (const std::exception& e) {
        return { rpc::OperationStatus::createFailed(std::string{ "Error occurred: " } + e.what()),
                 lk::Address::null(),
                 gas };
    }
}


std::tuple<rpc::OperationStatus, std::string, lk::Balance> GeneralServerService::transaction_message_call(
  lk::Balance amount,
  const lk::Address& from_address,
  const lk::Address& to_address,
  const base::Time& timestamp,
  lk::Balance gas,
  const std::string& hex_message,
  const lk::Sign& signature)
{
    LOG_TRACE << "Received RPC request {transaction_to_contract} with from_address[" << from_address.toString()
              << "], to_address[" << to_address.toString() << "], amount[" << amount << "], gas" << gas
              << "], timestamp[" << timestamp.getSecondsSinceEpoch() << "], message[" << hex_message << "]";


    auto message = base::fromHex<base::Bytes>(hex_message);

    lk::TransactionBuilder txb;
    txb.setType(lk::Transaction::Type::MESSAGE_CALL);
    txb.setFrom(from_address);
    txb.setTo(to_address);
    txb.setAmount(amount);
    txb.setFee(gas);
    txb.setTimestamp(timestamp);
    txb.setData(message);
    txb.setSign(signature);

    auto tx = std::move(txb).build();

    try {
        _core.addPendingTransactionAndWait(tx);
        auto hash = base::Sha256::compute(base::toBytes(tx));
        const auto& result_bytes = _core.getTransactionOutput(hash);
        if (result_bytes.isEmpty()) {
            return { rpc::OperationStatus::createFailed(std::string{ "Message call failed" }), std::string{}, gas };
        }
        base::SerializationIArchive ia(result_bytes);

        auto is_successful = ia.deserialize<bool>();
        if (!is_successful) {
            return { rpc::OperationStatus::createFailed(std::string{ "Message call failed" }), std::string{}, gas };
        }
        auto result = ia.deserialize<base::Bytes>();
        auto gas_left = ia.deserialize<lk::Balance>();
        return { rpc::OperationStatus::createSuccess("Message call was successfully executed"),
                 base::toHex<base::Bytes>(result),
                 gas_left };
    }
    catch (const std::exception& e) {
        return { rpc::OperationStatus::createFailed(std::string{ "Error occurred during message call: " } + e.what()),
                 std::string{},
                 gas };
    }
}

} // namespace node
