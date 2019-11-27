#include "grpc_adapter.hpp"

namespace rpc
{

GrpcAdapter::GrpcAdapter()
{}

void GrpcAdapter::init(std::shared_ptr<BaseRpc> service)
{
    _service = service;
}

::grpc::Status GrpcAdapter::balance(
    grpc::ServerContext* context, const likelib::Address* request, likelib::Money* response)
{
    auto address = request->address().c_str();

    try {
        response->set_money(_service->balance(address));
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}

::grpc::Status GrpcAdapter::transaction(
    grpc::ServerContext* context, const likelib::Transaction* request, likelib::Hash* response)
{
    auto from_address = request->from_address().address().c_str();
    auto to_address = request->to_address().address().c_str();
    auto amount = request->amount().money();
    auto creation_time = std::stoul(request->creation_time().milliseconds_from_epoch());

    try {
        response->set_hash_string(
            _service->transaction(amount, from_address, to_address, base::Time::fromMilliseconds(creation_time)));
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}

::grpc::Status GrpcAdapter::test(
    grpc::ServerContext* context, const likelib::TestRequest* request, likelib::TestResponse* response)
{
    auto data = request->message();
    try {
        response->set_message(_service->test(data));
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}

} // namespace rpc
