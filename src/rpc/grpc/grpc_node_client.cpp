#include "grpc_node_client.hpp"

#include "rpc/error.hpp"

rpc::GrpcNodeClient::GrpcNodeClient(const std::string& connect_address)
{
    auto channel_credentials = grpc::InsecureChannelCredentials();
    _stub = std::make_unique<likelib::Node::Stub>(grpc::CreateChannel(connect_address, channel_credentials));
}

bc::Balance rpc::GrpcNodeClient::balance(const bc::Address& address)
{
    // convert data for request
    likelib::Address request;
    request.set_address(address.toString());

    // call remote host
    likelib::Money reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->balance(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        auto result = reply.money();
        return result;
    }
    else {
        throw RpcError(status.error_message());
    }
}

std::string rpc::GrpcNodeClient::transaction(bc::Balance amount, const bc::Address& from_address,
    const bc::Address& to_address, const base::Time& transaction_time)
{
    // convert data for request
    auto request_amount = new likelib::Money();
    request_amount->set_money(static_cast<google::protobuf::uint64>(amount));

    auto request_from_address = new likelib::Address();
    request_from_address->set_address(from_address.toString());

    auto request_to_address = new likelib::Address;
    request_to_address->set_address(to_address.toString());

    auto request_transaction_time = new likelib::Time;
    request_transaction_time->set_seconds_from_epoch(std::to_string(transaction_time.seconds()));

    likelib::Transaction request;
    request.set_allocated_amount(request_amount);
    request.set_allocated_from_address(request_from_address);
    request.set_allocated_to_address(request_to_address);
    request.set_allocated_creation_time(request_transaction_time);

    // call remote host
    likelib::Hash reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->transaction(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        auto result = reply.hash_string();
        return result;
    }
    else {
        throw RpcError(status.error_message());
    }
}

std::string rpc::GrpcNodeClient::test(const std::string& test_request)
{
    // convert data for request
    likelib::TestRequest request;
    request.set_message(test_request);

    // call remote host
    likelib::TestResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->test(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        auto result = reply.message();
        return result;
    }
    else {
        throw RpcError(status.error_message());
    }
}
