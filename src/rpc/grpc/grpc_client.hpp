#pragma once

#include <public_rpc.grpc.pb.h>

#include "rpc/base_rpc.hpp"

#include <grpcpp/grpcpp.h>

namespace rpc::grpc
{

/// Class implementing connect to node by gRPC and call methods
class NodeClient final : public BaseRpc
{
  public:
    explicit NodeClient(const std::string& connect_address);

    ~NodeClient() override = default;

    lk::AccountInfo getAccountInfo(const lk::Address& address) override;

    Info getNodeInfo() override;

    lk::ImmutableBlock getBlock(const base::Sha256& block_hash) override;

    lk::ImmutableBlock getBlock(uint64_t block_number) override;

    lk::Transaction getTransaction(const base::Sha256& transaction_hash) override;

    lk::TransactionStatus pushTransaction(const lk::Transaction& transaction) override;

    lk::TransactionStatus getTransactionStatus(const base::Sha256& transaction_hash) override;

  private:
    std::unique_ptr<likelib::NodePublicInterface::Stub> _stub;
};

} // namespace rpc::grpc