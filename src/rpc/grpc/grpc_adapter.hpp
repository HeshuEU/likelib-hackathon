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
class GrpcAdapter final : public likelib::Node::Service
{
  public:
    /// default constructor
    explicit GrpcAdapter();

    /// default destructor
    ~GrpcAdapter() override = default;

    /// method that call init in LogicService instance was created by that
    void init(std::shared_ptr<BaseRpc> service);

  private:
    std::shared_ptr<BaseRpc> _service;

    ::grpc::Status balance(
        grpc::ServerContext* context, const likelib::Address* request, likelib::Money* response) override;

    ::grpc::Status transaction(
        grpc::ServerContext* context, const likelib::Transaction* request, likelib::Hash* response) override;

    ::grpc::Status test(
        grpc::ServerContext* context, const likelib::TestRequest* request, likelib::TestResponse* response) override;
};

} // namespace rpc
