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

    std::tuple<rpc::OperationStatus, bc::Address, bc::Balance> transaction_creation_contract(bc::Balance amount,
        const bc::Address& from_address, const base::Time& transaction_time, bc::Balance gas, const base::Bytes& code,
        const base::Bytes& initial_message, const bc::Sign& signature) override;

    std::tuple<rpc::OperationStatus, base::Bytes, bc::Balance> transaction_to_contract(bc::Balance amount,
        const bc::Address& from_address, const bc::Address& to_address, const base::Time& transaction_time,
        bc::Balance gas, const base::Bytes& message, const bc::Sign& signature) override;

    rpc::OperationStatus transaction_to_wallet(bc::Balance amount, const bc::Address& from_address,
        const bc::Address& to_address, const base::Time& transaction_time, bc::Balance fee, const bc::Sign& signature) override;

  private:
    lk::Core& _core;
};
} // namespace node
