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
        grpc::ServerContext* context, const likelib::Address* request, likelib::CurrencyAmount* response) override;

    grpc::Status info(grpc::ServerContext* context, const likelib::InfoRequest* request, likelib::InfoResponse* response) override;

    grpc::Status get_block(grpc::ServerContext* context, const likelib::GetBlockRequest* request, likelib::GetBlockResponse* response) override;

    grpc::Status message_call(grpc::ServerContext* context, const likelib::TransactionMessageCallRequest* request,
        likelib::TransactionMessageCallResponse* response) override;

    grpc::Status create_contract(grpc::ServerContext* context, const likelib::TransactionCreateContractRequest* request,
        likelib::TransactionCreateContractResponse* response) override;
};

} // namespace rpc
