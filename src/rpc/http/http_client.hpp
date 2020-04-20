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

    lk::AccountInfo get_account(const lk::Address& address) override;

    Info get_node_info() = 0;

    lk::Block get_block(const base::Sha256& block_hash) override;

    lk::Transaction get_transaction(const base::Sha256& transaction_hash) override;

    TransactionStatus push_transaction(lk::Transaction) override;

  private:
    web::http::client::http_client _client;
};

} // namespace rpc
