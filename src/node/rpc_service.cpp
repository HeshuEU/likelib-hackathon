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

std::string GeneralServerService::transaction(bc::Balance amount, const bc::Address& from_address,
    const bc::Address& to_address, const base::Time& transaction_time, const base::Bytes& sign)
{
    try {
        LOG_TRACE << "Node received in {transaction}: from_address[" << from_address.toString() << "], to_address["
                  << to_address.toString() << "], amount[" << amount << "], transaction_time["
                  << transaction_time.getSecondsSinceEpochBeginning() << "]";

        if(_core.performTransaction(bc::Transaction(from_address, to_address, amount, transaction_time))) {
            LOG_TRACE << "Added tx to pending";
            return "Success! Transaction added to queue successfully.";
        }
        else {
            LOG_TRACE << "Rejecting tx";
            return "Error! Transaction rejected.";
        }
    }
    catch(const std::exception& e) {
        LOG_WARNING << "Exception caught during transaction request: " << e.what();
        return "error";
    }
    catch(...) {
        LOG_WARNING << "Exception caught during transaction request: unknown";
        return "error";
    }
}

std::string GeneralServerService::test(const std::string& test_request)
{
    try {
        LOG_TRACE << "Node received in {test}: test_request[" << test_request << "]";
        auto our_request = base::Sha256::compute(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_REQUEST)).toHex();
        if(our_request == test_request) {
            return base::Sha256::compute(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_RESPONSE)).toHex();
        }
        else {
            return std::string();
        }
    }
    catch(const std::exception& e) {
        LOG_WARNING << "Exception caught during test request: " << e.what();
        return "error";
    }
    catch(...) {
        LOG_WARNING << "Exception caught during test request: unknown";
        return "error";
    }
}

} // namespace node
