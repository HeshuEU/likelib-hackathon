#include "general_server_service.hpp"

#include "base/log.hpp"

rpc::GeneralServerService::GeneralServerService() {
    LOG_TRACE << "Created GeneralServerService";
}

rpc::GeneralServerService::~GeneralServerService() {
    LOG_TRACE << "Deleted GeneralServerService";
}

void rpc::GeneralServerService::init(){
    LOG_TRACE << "Inited GeneralServerService";
}

bc::Balance rpc::GeneralServerService::balance(const bc::Address &address) {
    LOG_TRACE << "Node received in {balance}: address[" << address.toString() << "]";
    return 0;
}

std::string rpc::GeneralServerService::transaction(bc::Balance amount, const bc::Address &from_address,
                                              const bc::Address &to_address) {
    LOG_TRACE << "Node received in {transaction}: from_address[" << from_address.toString()
              << "], to_address[" << to_address.toString() << "], amount["
              << amount << "]";
    return "likelib";
}