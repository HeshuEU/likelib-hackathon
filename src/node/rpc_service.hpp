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

    ~GeneralServerService() override = default;

    lk::AccountInfo getAccountInfo(const lk::Address& address) override;

    rpc::Info getNodeInfo() override;

    lk::ImmutableBlock getBlock(const base::Sha256& block_hash) override;

    lk::ImmutableBlock getBlock(uint64_t block_number) override;

    lk::Transaction getTransaction(const base::Sha256& transaction_hash) override;

    lk::TransactionStatus pushTransaction(const lk::Transaction& transaction) override;

    lk::TransactionStatus getTransactionStatus(const base::Sha256& transaction_hash) override;

  private:
    lk::Core& _core;
};

} // namespace node
