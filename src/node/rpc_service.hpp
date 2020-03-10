#pragma once

#include "bc/transaction.hpp"
#include "lk/core.hpp"
#include "rpc/base_rpc.hpp"

namespace node
{

class GeneralServerService : public rpc::BaseRpc
{
  public:
    explicit GeneralServerService(lk::Core& core);

    ~GeneralServerService() override;

    rpc::OperationStatus test(uint32_t api_version) override;

    bc::Balance balance(const bc::Address& address) override;

    rpc::Info info() override;

    std::tuple<rpc::OperationStatus, bc::Address, bc::Balance> transaction_create_contract(bc::Balance amount,
        const bc::Address& from_address, const base::Time& timestamp, bc::Balance gas, const std::string& contract_code,
        const std::string& init, const bc::Sign& signature) override;

    std::tuple<rpc::OperationStatus, std::string, bc::Balance> transaction_message_call(bc::Balance amount,
        const bc::Address& from_address, const bc::Address& to_address, const base::Time& timestamp, bc::Balance gas,
        const std::string& message, const bc::Sign& signature) override;

  private:
    lk::Core& _core;
};
} // namespace node
