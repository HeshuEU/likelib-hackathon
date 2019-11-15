#pragma once

#include "gen/public_rpc.grpc.pb.h"

#include "bc/base_service.hpp"

#include "base/log.hpp"
#include "base/error.hpp"
#include "base/config.hpp"

#include <grpcpp/grpcpp.h>

namespace rpc
{

/// Class implement receive gRPC messages and call similar method from LogicService instance and send answers or error
/// messages \tparam LogicService blockchain logic interface class implimented from bc::BaseService
template<typename LogicService>
class GrpcNodeServiceImpl final : public likelib::Node::Service
{
  public:
    /// default constructor
    explicit GrpcNodeServiceImpl() : _service{}
    {}

    /// default destructor
    ~GrpcNodeServiceImpl() override = default;

    /// method that call init in LogicService instance was created by that
    void init()
    {
        _service.init();
    }

  private:
    LogicService _service;

    static constexpr const char* LOG_ID = " |GRPC SERVICE| ";

    ::grpc::Status balance(grpc::ServerContext* context, const likelib::Address* request,
                           likelib::Money* response) override
    {
        auto address = request->address().c_str();

        try {
            response->set_money(_service.balance(address));
        }
        catch(const base::Error& e) {
            LOG_ERROR << LOG_ID << e.what();
            return ::grpc::Status::CANCELLED;
        }
        return ::grpc::Status::OK;
    }

    ::grpc::Status transaction(grpc::ServerContext* context, const likelib::Transaction* request,
                               likelib::Hash* response) override
    {
        auto from_address = request->from_address().address().c_str();
        auto to_address = request->to_address().address().c_str();
        auto amount = request->amount().money();

        try {
            response->set_hash_string(_service.transaction(amount, from_address, to_address));
        }
        catch(const base::Error& e) {
            LOG_ERROR << LOG_ID << e.what();
            return ::grpc::Status::CANCELLED;
        }

        return ::grpc::Status::OK;
    }
};

} // namespace rpc
