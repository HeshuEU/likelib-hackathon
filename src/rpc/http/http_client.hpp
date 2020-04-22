#pragma once

#include <rpc/base_rpc.hpp>

#include <cpprest/http_client.h>

#include <string>

namespace rpc::http
{

/// Class implementing connect to node by gRPC and call methods
class NodeClient final : public BaseRpc
{
  public:
    explicit NodeClient(const std::string& connect_address);

    ~NodeClient() override = default;

    lk::AccountInfo getAccount(const lk::Address& address) override;

    Info getNodeInfo() override;

    lk::Block getBlock(const base::Sha256& block_hash) override;

    lk::Transaction getTransaction(const base::Sha256& transaction_hash) override;

    TransactionStatus pushTransaction(const lk::Transaction& transaction) override;

    virtual TransactionStatus getTransactionResult(const base::Sha256& transaction_hash) override;

  private:
    web::http::client::http_client _client;
};

} // namespace rpc
