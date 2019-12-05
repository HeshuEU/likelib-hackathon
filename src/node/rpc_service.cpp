#include "rpc_service.hpp"

#include "base/log.hpp"
#include "base/hash.hpp"
#include "base/config.hpp"
#include "bc/transaction.hpp"

namespace node
{

GeneralServerService::GeneralServerService(bc::Blockchain* bc) : _bc{bc}
{
    LOG_TRACE << "Created GeneralServerService";
}

GeneralServerService::~GeneralServerService()
{
    LOG_TRACE << "Deleted GeneralServerService";
}

bc::Balance GeneralServerService::balance(const bc::Address& address)
{
    LOG_TRACE << "Node received in {balance}: address[" << address.toString() << "]";
    return _bc->getBalance(address);
}

std::string GeneralServerService::transaction(bc::Balance amount, const bc::Address& from_address,
    const bc::Address& to_address, const base::Time& transaction_time)
{
    LOG_TRACE << "Node received in {transaction}: from_address[" << from_address.toString() << "], to_address["
              << to_address.toString() << "], amount[" << amount << "], transaction_time["
              << transaction_time.secondsInEpoch() << "]";

    if(_bc->processReceivedTransaction(bc::Transaction(from_address, to_address, amount, transaction_time))) {
        return "Transaction was received and added to queue.";
    }
    else {
        return "Transaction was not approved";
    }
}

std::string GeneralServerService::test(const std::string& test_request)
{
    LOG_TRACE << "Node received in {test}: test_request[" << test_request << "]";
    auto our_request = base::Sha256::compute(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_REQUEST)).toHex();
    if(our_request == test_request) {
        return base::Sha256::compute(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_RESPONSE)).toHex();
    }
    else {
        return std::string();
    }
}

} // namespace node