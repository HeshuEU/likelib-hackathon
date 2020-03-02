#include "rpc_service.hpp"

#include "base/log.hpp"
#include "base/hash.hpp"
#include "base/config.hpp"
#include "bc/transaction.hpp"

namespace node
{

GeneralServerService::GeneralServerService(lk::Core& core) : _core{core}
{
    LOG_TRACE << "Created GeneralServerService";
}


GeneralServerService::~GeneralServerService()
{
    LOG_TRACE << "Deleted GeneralServerService";
}


rpc::OperationStatus GeneralServerService::test(uint32_t api_version)
{
    LOG_TRACE << "Node received in {test}: test_request[" << api_version << "]";
    if(base::config::RPC_PUBLIC_API_VERSION == api_version) {
        return rpc::OperationStatus::createSuccess("RPC api is compatible");
    }
    else {
        if(base::config::RPC_PUBLIC_API_VERSION > api_version) {
            return rpc::OperationStatus::createFailed("Your RPC api is old.");
        }
        return rpc::OperationStatus::createFailed("Not support api version");
    }
}



bc::Balance GeneralServerService::balance(const bc::Address& address)
{
    try {
        LOG_TRACE << "Node received in {balance}: address[" << address.toString() << "]";
        auto ret = _core.getBalance(address);
        return ret;
    }
    catch(const std::exception& e) {
        LOG_WARNING << "Exception caught during balance request: " << e.what();
        return -1;
    }
    catch(...) {
        LOG_WARNING << "Exception caught during balance request: unknown";
        return -1;
    }
}

std::tuple<rpc::OperationStatus, bc::Address, bc::Balance> GeneralServerService::transaction_create_contract(
    bc::Balance amount, const bc::Address& from_address, const base::Time& timestamp, bc::Balance gas,
    const std::string& hex_contract_code, const std::string& hex_init, const bc::Sign& signature)
{
    LOG_TRACE << "Node received in {transaction_to_contract}: from_address[" << from_address.toString() << "], amount["
              << amount << "], gas" << gas << "], code[" << hex_contract_code << "], timestamp["
              << timestamp.getSecondsSinceEpochBeginning() << "], initial_message[" << hex_init << "]";

    auto contract_code = base::Bytes::fromHex(hex_contract_code);
    auto init = base::Bytes::fromHex(hex_init);

    bc::TransactionBuilder txb;
    txb.setTransactionType(bc::Transaction::Type::CONTRACT_CREATION);
    txb.setFrom(from_address);
    txb.setTo(from_address); // just a placeholder. TODO: a better thing, maybe NULL-address
    txb.setAmount(amount);
    txb.setFee(gas);
    txb.setTimestamp(timestamp);

    bc::ContractInitData data(std::move(contract_code), std::move(init));
    txb.setData(base::toBytes(data));

    txb.setSign(signature);

    auto tx = std::move(txb).build();

    try {
        auto contract_address = _core.createContract(tx);
        return {rpc::OperationStatus::createSuccess("Contract was successfully deployed"), contract_address, gas};
    }
    catch(const std::exception& e) {
        return {rpc::OperationStatus::createFailed(std::string{"Error occurred"} + e.what()), bc::Address::null(), gas};
    }
}


std::tuple<rpc::OperationStatus, std::string, bc::Balance> GeneralServerService::transaction_message_call(
    bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address,
    const base::Time& timestamp, bc::Balance gas, const std::string& hex_message, const bc::Sign& signature)
{
    LOG_TRACE << "Node received in {transaction_to_contract}: from_address[" << from_address.toString()
              << "], to_address[" << to_address.toString() << "], amount[" << amount << "], gas" << gas
              << "], timestamp[" << timestamp.getSecondsSinceEpochBeginning() << "], message["
              << hex_message << "]";


    auto message = base::Bytes::fromHex(hex_message);

    bc::TransactionBuilder txb;
    txb.setTransactionType(bc::Transaction::Type::MESSAGE_CALL);
    txb.setFrom(from_address);
    txb.setTo(to_address);
    txb.setAmount(amount);
    txb.setFee(gas);
    txb.setTimestamp(timestamp);
    txb.setData(message);
    txb.setSign(signature);

    auto tx = std::move(txb).build();

    try {
        auto result = _core.messageCall(tx);
        return {rpc::OperationStatus::createSuccess("Message call was successfully executed"), result.toOutputData().toHex(), result.gasLeft()};
    }
    catch(const std::exception& e) {
        return {rpc::OperationStatus::createFailed(std::string{"Error occurred during message call: "} + e.what()), std::string{}, gas};
    }
}

} // namespace node
