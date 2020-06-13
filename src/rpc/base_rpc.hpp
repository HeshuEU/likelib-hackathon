#pragma once

#include "core/block.hpp"
#include "core/core.hpp"
#include "core/managers.hpp"
#include "core/transaction.hpp"
#include "core/types.hpp"

namespace rpc
{

struct Info
{
    base::Sha256 top_block_hash;
    uint64_t top_block_number;
    uint32_t api_version;
};


class BaseRpc
{
  public:
    virtual ~BaseRpc() = default;

    virtual lk::AccountInfo getAccountInfo(const lk::Address& address) = 0;

    virtual Info getNodeInfo() = 0;

    virtual lk::ImmutableBlock getBlock(const base::Sha256& block_hash) = 0;

    virtual lk::ImmutableBlock getBlock(uint64_t block_number) = 0;

    virtual lk::Transaction getTransaction(const base::Sha256& transaction_hash) = 0;

    virtual lk::TransactionStatus pushTransaction(const lk::Transaction& transaction) = 0;

    virtual lk::TransactionStatus getTransactionStatus(const base::Sha256& transaction_hash) = 0;
};

} // namespace rpc
