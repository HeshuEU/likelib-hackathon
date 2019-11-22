#include "grpc_service.hpp"

namespace rpc
{

GrpcNodeServiceImpl::GrpcNodeServiceImpl()
{}

void GrpcNodeServiceImpl::init(std::shared_ptr<bc::BaseService> service)
{
    _service = service;
}

::grpc::Status GrpcNodeServiceImpl::balance(
    grpc::ServerContext* context, const likelib::Address* request, likelib::Money* response)
{
    auto address = request->address().c_str();

    try {
        response->set_money(_service->balance(address));
    }
    catch(const base::Error& e) {
        LOG_ERROR << LOG_ID << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}

::grpc::Status GrpcNodeServiceImpl::transaction(
    grpc::ServerContext* context, const likelib::Transaction* request, likelib::Hash* response)
{
    auto from_address = request->from_address().address().c_str();
    auto to_address = request->to_address().address().c_str();
    auto amount = request->amount().money();

    try {
        response->set_hash_string(_service->transaction(amount, from_address, to_address));
    }
    catch(const base::Error& e) {
        LOG_ERROR << LOG_ID << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}

::grpc::Status GrpcNodeServiceImpl::test(
    grpc::ServerContext* context, const likelib::TestRequest* request, likelib::TestResponse* response)
{
    auto data = request->message();
    try {
        response->set_message(_service->test(data));
    }
    catch(const base::Error& e) {
        LOG_ERROR << LOG_ID << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}

} // namespace rpc
