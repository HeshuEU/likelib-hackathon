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
    bc::Balance amount, const bc::Address& from_address, const base::Time& transaction_time, bc::Balance gas,
    const std::string& hex_contract_code, const std::string& hex_init, const bc::Sign& signature)
{
    LOG_TRACE << "Node received in {transaction_to_contract}: from_address[" << from_address.toString() << "], amount["
              << amount << "], gas" << gas << "], code[" << hex_contract_code << "], transaction_time["
              << transaction_time.getSecondsSinceEpochBeginning() << "], initial_message[" << hex_init << "]";

    auto contract_code = base::Bytes::fromHex(hex_contract_code);
    auto init = base::Bytes::fromHex(hex_init);

    bc::TransactionBuilder txb;
    txb.setTransactionType(bc::Transaction::Type::CONTRACT_CREATION);
    txb.setFrom(from_address);
    // txb.setTo(SOME_NULL_ADDRESS)?
    txb.setAmount(amount);
    txb.setFee(gas);

    bc::ContractInitData data(std::move(contract_code), std::move(init));
    txb.setData(base::toBytes(data));

    txb.setSign(signature);

    auto tx = std::move(txb).build();

    if(_core.performTransaction(tx)) {
        return {rpc::OperationStatus::createSuccess("Contract was successfully deployed"), bc::Address{}, gas};
    }
    else {
        return {rpc::OperationStatus::createFailed("Function is not supported"), bc::Address{}, gas};
    }
}


std::tuple<rpc::OperationStatus, std::string, bc::Balance> GeneralServerService::transaction_message_call(
    bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address,
    const base::Time& transaction_time, bc::Balance gas, const std::string& hex_message, const bc::Sign& signature)
{
    LOG_TRACE << "Node received in {transaction_to_contract}: from_address[" << from_address.toString()
              << "], to_address[" << to_address.toString() << "], amount[" << amount << "], gas" << gas
              << "], transaction_time[" << transaction_time.getSecondsSinceEpochBeginning() << "], message["
              << hex_message << "]";


    auto message = base::Bytes::fromHex(hex_message);

    bc::TransactionBuilder txb;
    txb.setTransactionType(bc::Transaction::Type::MESSAGE_CALL);
    txb.setFrom(from_address);
    txb.setTo(to_address);
    txb.setAmount(amount);
    txb.setFee(gas);
    txb.setData(message);
    txb.setSign(signature);

    auto tx = std::move(txb).build();

    if(_core.performTransaction(tx)) {
        return {rpc::OperationStatus::createSuccess("Message call was successfully executed"), std::string{}, gas};
    }
    else {
        return {rpc::OperationStatus::createFailed("Function is not support"), std::string{}, gas};
    }
}

} // namespace node
