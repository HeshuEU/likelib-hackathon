#include "general_server_service.hpp"

#include "base/log.hpp"

bc::GeneralServerService::GeneralServerService() {
    LOG_TRACE << "Created GeneralServerService";
}

bc::GeneralServerService::~GeneralServerService() {
    LOG_TRACE << "Deleted GeneralServerService";
}

void bc::GeneralServerService::init(){
    LOG_TRACE << "Inited GeneralServerService";
}

bc::Balance bc::GeneralServerService::balance(const bc::Address &address) {
    LOG_TRACE << "Node received in {balance}: address[" << address.toString() << "]";
    return 0;
}

std::string bc::GeneralServerService::transaction(bc::Balance amount, const bc::Address &from_address,
                                              const bc::Address &to_address) {
    LOG_TRACE << "Node received in {transaction}: from_address[" << from_address.toString()
              << "], to_address[" << to_address.toString() << "], amount["
              << amount << "]";
    return "likelib";
}