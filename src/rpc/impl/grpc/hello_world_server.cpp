#include "rpc/impl/grpc/gen/public_rpc.grpc.pb.h"

#include "base/log.hpp"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>


class NodeServiceImpl final : public Likelib::Node::Service {
    grpc::Status
    balance(grpc::ServerContext *context, const Likelib::Address *request, Likelib::Money *response) override {
        LOG_INFO << "Node was call in {balance}";
        logContextData(context);
        LOG_INFO << "Node received in {balance}: address[" << request->address().c_str() << "]";
        response->set_money(0);
        return grpc::Status::OK;
    }

    grpc::Status
    transaction(grpc::ServerContext *context, const Likelib::Transaction *request, Likelib::Hash *response) override {
        LOG_INFO << "Node was call in {transaction}";
        logContextData(context);
        LOG_INFO << "Node received in {transaction}: from_address[" << request->from_address().address().c_str()
                 << "], to_address[" << request->to_address().address().c_str() << "], amount["
                 << request->amount().money() << "]";
        response->set_hash_string("likelib");
        return grpc::Status::OK;
    }

    static void logContextData(grpc::ServerContext *context) {
        LOG_INFO << "Peer:" << context->peer();
        LOG_INFO << "----------- Client metadata -----------";
        for (auto& pair : context->client_metadata()) {
            LOG_INFO << "Key[" << pair.first.data() << "], value [" << pair.second.data() << "]";
        }
    }
};


std::unique_ptr<grpc::Server> makeServer(const std::string &server_address, grpc::Service &service) {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    return std::move(builder.BuildAndStart());
}


int main(int argc, char **argv) {
    std::string server_address("0.0.0.0:50051");
    NodeServiceImpl nodeService;
    std::unique_ptr<grpc::Server> server = makeServer(server_address, nodeService);
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
    return 0;
}