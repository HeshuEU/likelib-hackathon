#pragma once

#include <public_rpc.grpc.pb.h>

#include "rpc/base_rpc.hpp"

#include <grpcpp/grpcpp.h>

#include <memory>

namespace rpc::grpc
{

/// Class implement receive gRPC messages and call similar method from LogicService instance and send answers or error
/// messages
class Adapter final : public likelib::NodePublicInterface::Service
{
  public:
    /// default constructor
    explicit Adapter() = default;

    /// default destructor
    ~Adapter() override = default;

    /// method that call init in LogicService instance was created by that
    void init(std::shared_ptr<BaseRpc> service);

  private:
    std::shared_ptr<BaseRpc> _service;

    ::grpc::Status get_account(::grpc::ServerContext* context,
                               const ::likelib::Address* request,
                               ::likelib::AccountInfo* response) override;

    ::grpc::Status get_node_info(::grpc::ServerContext* context,
                                 [[maybe_unused]] const ::likelib::None* request,
                                 ::likelib::NodeInfo* response) override;

    ::grpc::Status get_block_by_hash(::grpc::ServerContext* context,
                                     const ::likelib::Hash* request,
                                     ::likelib::Block* response) override;

    ::grpc::Status get_block_by_number(::grpc::ServerContext* context,
                                       const ::likelib::Number* request,
                                       ::likelib::Block* response) override;

    ::grpc::Status get_transaction(::grpc::ServerContext* context,
                                   const ::likelib::Hash* request,
                                   ::likelib::Transaction* response) override;

    ::grpc::Status push_transaction(::grpc::ServerContext* context,
                                    const ::likelib::Transaction* request,
                                    ::likelib::TransactionStatus* response) override;


    ::grpc::Status get_transaction_result(::grpc::ServerContext* context,
                                          const ::likelib::Hash* request,
                                          ::likelib::TransactionStatus* response) override;
};


} // namespace rpc::grpc
