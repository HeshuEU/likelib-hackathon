#include "rpc_service.hpp"

#include "base/log.hpp"
#include "base/hash.hpp"
#include "base/config.hpp"

namespace node
{

GeneralServerService::GeneralServerService()
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
    return 0;
}

std::string GeneralServerService::transaction(
    bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address)
{
    LOG_TRACE << "Node received in {transaction}: from_address[" << from_address.toString() << "], to_address["
              << to_address.toString() << "], amount[" << amount << "]";
    return "likelib";
}


std::string GeneralServerService::test(const std::string& test_request)
{
    LOG_TRACE << "Node received in {test}: test_request[" << test_request << "]";
    //    auto our_request = base::sha256(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_REQUEST)).toString();

    auto our_request = std::string(base::config::RPC_CURRENT_SECRET_TEST_REQUEST);

    if(our_request == test_request) {
        //        return base::sha256(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_RESPONSE)).toString();
        return std::string(base::config::RPC_CURRENT_SECRET_TEST_RESPONSE);
    }
    else {
        return std::string();
    }
}

} // namespace node