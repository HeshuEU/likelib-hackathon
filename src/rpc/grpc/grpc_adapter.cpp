#include "grpc_adapter.hpp"

namespace rpc
{

GrpcAdapter::GrpcAdapter()
{}

void GrpcAdapter::init(std::shared_ptr<BaseRpc> service)
{
    _service = service;
}

::grpc::Status GrpcAdapter::balance(grpc::ServerContext*, const likelib::Address* request, likelib::Money* response)
{
    auto base64_address = request->address();

    try {
        response->set_money(_service->balance(bc::Address{base64_address}));
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}

::grpc::Status GrpcAdapter::transaction(
    grpc::ServerContext*, const likelib::Transaction* request, likelib::Hash* response)
{
    auto from_address = bc::Address(request->from_address().address());
    auto to_address = bc::Address(request->to_address().address());
    auto amount = request->amount().money();

    auto creation_time = static_cast<std::uint_least32_t>(std::stoul(request->creation_time().seconds_from_epoch()));

    auto sign = base::Bytes(request->sign().hex_sign());
    try {
        response->set_hash_string(_service->transaction(
            amount, from_address, to_address, base::Time::fromSecondsSinceEpochBeginning(creation_time), sign));
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}

::grpc::Status GrpcAdapter::test(
    grpc::ServerContext*, const likelib::TestRequest* request, likelib::TestResponse* response)
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
