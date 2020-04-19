#pragma once

#include "core/core.hpp"
#include "core/transaction.hpp"
#include "rpc/base_rpc.hpp"

namespace node
{

class GeneralServerService : public rpc::BaseRpc
{
  public:
    explicit GeneralServerService(lk::Core& core);

    ~GeneralServerService() override;

    rpc::OperationStatus test(uint32_t api_version) override;

    std::string balance(const lk::Address& address) override;

    rpc::Info info() override;

    lk::Block get_block(const base::Sha256& block_hash) override;

    std::tuple<rpc::OperationStatus, lk::Address, std::uint64_t> transaction_create_contract(
      lk::Balance amount,
      const lk::Address& from_address,
      const base::Time& timestamp,
      std::uint64_t gas,
      const std::string& contract_code,
      const std::string& init,
      const lk::Sign& signature) override;

    std::tuple<rpc::OperationStatus, std::string, std::uint64_t> transaction_message_call(
      lk::Balance amount,
      const lk::Address& from_address,
      const lk::Address& to_address,
      const base::Time& timestamp,
      std::uint64_t gas,
      const std::string& message,
      const lk::Sign& signature) override;

  private:
    lk::Core& _core;
};
} // namespace node
