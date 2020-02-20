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

std::tuple<rpc::OperationStatus, bc::Address, bc::Balance> GeneralServerService::transaction_creation_contract(
    bc::Balance amount, const bc::Address& from_address, const base::Time& transaction_time, bc::Balance gas,
    const base::Bytes& code, const base::Bytes& initial_message, const bc::Sign& signature)
{
    LOG_TRACE << "Node received in {transaction_to_contract}: from_address[" << from_address.toString() << "], amount["
              << amount << "], gas" << gas << "], code[" << code.toHex() << "], transaction_time["
              << transaction_time.getSecondsSinceEpochBeginning() << "], initial_message[" << initial_message.toHex()
              << "]";

    return {rpc::OperationStatus::createFailed("Function is not supported"), bc::Address{}, gas};
}

std::tuple<rpc::OperationStatus, base::Bytes, bc::Balance> GeneralServerService::transaction_to_contract(
    bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address,
    const base::Time& transaction_time, bc::Balance gas, const base::Bytes& message, const bc::Sign& signature)
{
    LOG_TRACE << "Node received in {transaction_to_contract}: from_address[" << from_address.toString()
              << "], to_address[" << to_address.toString() << "], amount[" << amount << "], gas" << gas
              << "], transaction_time[" << transaction_time.getSecondsSinceEpochBeginning() << "], message["
              << message.toHex() << "]";

    return {rpc::OperationStatus::createFailed("Function is not support"), base::Bytes{}, gas};
}

rpc::OperationStatus GeneralServerService::transaction_to_wallet(bc::Balance amount, const bc::Address& from_address,
    const bc::Address& to_address, const base::Time& transaction_time, bc::Balance fee, const bc::Sign& signature)
{
    LOG_TRACE << "Node received in {transaction_to_wallet}: from_address[" << from_address.toString()
              << "], to_address[" << to_address.toString() << "], amount[" << amount << "], fee" << fee
              << "], transaction_time[" << transaction_time.getSecondsSinceEpochBeginning() << "]";

    if(_core.performTransaction(bc::Transaction(from_address, to_address, amount, transaction_time, fee, signature))) {
        return rpc::OperationStatus::createSuccess("Success! Transaction added to queue successfully.");
    }
    else {
        return rpc::OperationStatus::createFailed("Error! Transaction rejected.");
    }
}

} // namespace node
