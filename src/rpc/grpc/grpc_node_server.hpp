#pragma once

#include "grpc_service.hpp"

namespace rpc {

    template<typename LogicService>
    class GrpcNodeServer {
    public:
        explicit GrpcNodeServer(const std::string &server_address) : _service{}, _server_address(server_address) {
            _service.init();
        }

        ~GrpcNodeServer(){
            stop();
        }

        void run() {
            grpc::ServerBuilder builder;
            builder.AddListeningPort(_server_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&_service);
            _server = builder.BuildAndStart();
        }

        void stop() {
            _server->Shutdown();
        }

    private:
        const std::string _server_address;
        std::unique_ptr<grpc::Server> _server = nullptr;
        GrpcNodeServiceImpl<LogicService> _service;
    };

}