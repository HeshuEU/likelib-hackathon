#include "grpc_node_client.hpp"

#include "base/log.hpp"
#include "base/error.hpp"


rpc::GrpcNodeClient::GrpcNodeClient(const std::string &connect_address) {
    _stub = std::make_unique<Likelib::Node::Stub>(
            grpc::CreateChannel(connect_address, grpc::InsecureChannelCredentials()));
}


bc::Balance rpc::GrpcNodeClient::balance(const bc::Address &address) {
    // convert data for request
    Likelib::Address request;
    request.set_address(address.toString());

    // call remote host
    Likelib::Money reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->balance(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        auto result = reply.money();
        return result;
    } else {
        throw base::Error{status.error_message()};
    }
}


std::string rpc::GrpcNodeClient::transaction(bc::Balance amount, const bc::Address &from_address,
                                             const bc::Address &to_address) {
    // convert data for request
    auto request_amount = new Likelib::Money();
    request_amount->set_money(static_cast<google::protobuf::uint64>(amount));

    auto request_from_address = new Likelib::Address();
    request_from_address->set_address(from_address.toString());

    auto request_to_address = new Likelib::Address;
    request_to_address->set_address(to_address.toString());

    Likelib::Transaction request;
    request.set_allocated_amount(request_amount);
    request.set_allocated_from_address(request_from_address);
    request.set_allocated_to_address(request_to_address);

    // call remote host
    Likelib::Hash reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->transaction(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        auto result = reply.hash_string();
        return result;
    } else {
        throw base::Error{status.error_message()};
    }
}
