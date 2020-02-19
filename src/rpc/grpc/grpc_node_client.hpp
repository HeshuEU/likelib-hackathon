#pragma once

#include "base/time.hpp"
#include "bc/address.hpp"
#include "bc/transaction.hpp"
#include "rpc/base_rpc.hpp"

#include <public_rpc.grpc.pb.h>
#include <grpcpp/grpcpp.h>

#include <string>

namespace rpc
{

/// Class implementing connect to node by gRPC and call methods
class GrpcNodeClient final : BaseRpc
{
  public:
    explicit GrpcNodeClient(const std::string& connect_address);

    ~GrpcNodeClient() override = default;

    bc::Balance balance(const bc::Address& address) override;

    std::string transaction(bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address,
        const base::Time& transaction_time, const std::string& base64_sign) override;

    std::string test(const std::string& test_request) override;

  private:
    std::unique_ptr<likelib::Node::Stub> _stub;
};

} // namespace rpc