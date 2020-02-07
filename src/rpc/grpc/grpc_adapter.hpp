#pragma once

#include <public_rpc.grpc.pb.h>

#include "rpc/base_rpc.hpp"

#include "base/log.hpp"
#include "base/error.hpp"
#include "base/config.hpp"

#include <grpcpp/grpcpp.h>

#include <memory>

namespace rpc
{

/// Class implement receive gRPC messages and call similar method from LogicService instance and send answers or error
/// messages
class GrpcAdapter final : public likelib::NodePublicInterface::Service
{
  public:
    /// default constructor
    explicit GrpcAdapter() = default;

    /// default destructor
    ~GrpcAdapter() override = default;

    /// method that call init in LogicService instance was created by that
    void init(std::shared_ptr<BaseRpc> service);

  private:
    std::shared_ptr<BaseRpc> _service;

    grpc::Status test(
        grpc::ServerContext* context, const likelib::TestRequest* request, likelib::TestResponse* response) override;

    grpc::Status balance(
        grpc::ServerContext* context, const likelib::Address* request, likelib::Money* response) override;

    grpc::Status transaction_to_contract(grpc::ServerContext* context,
        const likelib::TransactionToContractRequest* request,
        likelib::TransactionToContractResponse* response) override;

    grpc::Status transaction_for_create_contract(grpc::ServerContext* context,
        const likelib::TransactionCreationContractRequest* request,
        likelib::TransactionCreationContractResponse* response) override;

    grpc::Status transaction_to_wallet(grpc::ServerContext* context,
        const likelib::TransactionToAccountRequest* request, likelib::TransactionToAccountResponse* response) override;
};

} // namespace rpc
