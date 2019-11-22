#pragma once
#include "grpc_service.hpp"

namespace rpc
{
template<typename LogicService>
GrpcNodeServiceImpl<LogicService>::GrpcNodeServiceImpl() : _service{}
{}

template<typename LogicService>
void GrpcNodeServiceImpl<LogicService>::init()
{
    _service.init();
}

template<typename LogicService>
::grpc::Status GrpcNodeServiceImpl<LogicService>::balance(grpc::ServerContext* context, const likelib::Address* request,
                                                          likelib::Money* response)
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

template<typename LogicService>
::grpc::Status GrpcNodeServiceImpl<LogicService>::transaction(grpc::ServerContext* context,
                                                              const likelib::Transaction* request,
                                                              likelib::Hash* response)
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

template<typename LogicService>
::grpc::Status GrpcNodeServiceImpl<LogicService>::test(grpc::ServerContext* context,
                                                       const likelib::TestRequest* request,
                                                       likelib::TestResponse* response)
{
    auto data = request->message();
    try {
        response->set_message(_service.test(data));
    }
    catch(const base::Error& e) {
        LOG_ERROR << LOG_ID << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}

} // namespace rpc