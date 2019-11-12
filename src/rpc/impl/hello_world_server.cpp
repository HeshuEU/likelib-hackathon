#include "gen/public_rpc.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(ServerContext *context, const HelloRequest *request,
                    HelloReply *reply) override {
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return Status::OK;
    }
};


std::unique_ptr<Server> makeServer(const std::string &server_address, GreeterServiceImpl &service) {
    ServerBuilder builder;
// Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
// Register "service" as the instance through which we'll communicate with
// clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
// Finally assemble the server.
    return std::move(builder.BuildAndStart());
}


int main(int argc, char **argv) {
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;
    std::unique_ptr<Server> server = makeServer(server_address, service);
    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}