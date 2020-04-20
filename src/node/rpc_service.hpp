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

    lk::AccountInfo get_account(const lk::Address& address) override;

    rpc::Info get_node_info() override;

    lk::Block get_block(const base::Sha256& block_hash) override;

    lk::Transaction get_transaction(const base::Sha256& transaction_hash) override;

    rpc::TransactionStatus push_transaction(lk::Transaction tx) override;

  private:
    lk::Core& _core;
};
} // namespace node
