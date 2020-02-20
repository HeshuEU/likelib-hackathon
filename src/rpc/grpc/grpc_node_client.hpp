#pragma once

#include <public_rpc.grpc.pb.h>

#include "rpc/base_rpc.hpp"

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

    OperationStatus test(uint32_t api_version) override;

    bc::Balance balance(const bc::Address& address) override;

    std::tuple<OperationStatus, bc::Address, bc::Balance> transaction_creation_contract(bc::Balance amount,
        const bc::Address& from_address, const base::Time& transaction_time,
        bc::Balance gas, const base::Bytes& code, const base::Bytes& initial_message) override;

    std::tuple<OperationStatus, base::Bytes, bc::Balance> transaction_to_contract(bc::Balance amount,
        const bc::Address& from_address, const bc::Address& to_address, const base::Time& transaction_time,
        bc::Balance gas, const base::Bytes& message) override;

    OperationStatus transaction_to_wallet(bc::Balance amount, const bc::Address& from_address,
        const bc::Address& to_address, bc::Balance fee, const base::Time& transaction_time) override;

  private:
    std::unique_ptr<likelib::NodePublicInterface::Stub> _stub;
};

