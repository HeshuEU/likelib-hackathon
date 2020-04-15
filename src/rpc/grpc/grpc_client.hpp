#pragma once

#include <public_rpc.grpc.pb.h>

#include <rpc/base_rpc.hpp>

#include <grpcpp/grpcpp.h>

#include <string>

namespace rpc
{
namespace grpc
{

/// Class implementing connect to node by gRPC and call methods
class NodeClient final : public BaseRpc
{
  public:
    explicit NodeClient(const std::string& connect_address);

    ~NodeClient() override = default;

    uint32_t get_api_version() override;

    lk::Balance balance(const lk::Address& address) override;

    Info info() override;

    lk::Block get_block(const base::Sha256& block_hash) override;

    std::tuple<OperationStatus, lk::Address, lk::Balance> transaction_create_contract(
      lk::Balance amount,
      const lk::Address& from_address,
      const base::Time& transaction_time,
      lk::Balance gas,
      const std::string& contract_code,
      const std::string& init,
      const lk::Sign& signature) override;

    std::tuple<OperationStatus, std::string, lk::Balance> transaction_message_call(lk::Balance amount,
                                                                                   const lk::Address& from_address,
                                                                                   const lk::Address& to_address,
                                                                                   const base::Time& transaction_time,
                                                                                   lk::Balance fee,
                                                                                   const std::string& data,
                                                                                   const lk::Sign& signature) override;

  private:
    std::unique_ptr<likelib::NodePublicInterface::Stub> _stub;
};

}
} // namespace rpc