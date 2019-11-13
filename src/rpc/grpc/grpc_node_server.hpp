#pragma once

#include "grpc_service.hpp"

namespace rpc {

    /// Template server was implemented logic to start listening messages by gRPC
    /// \tparam LogicService blockchain logic interface class implemented from bc::BaseService
    template<typename LogicService>
    class GrpcNodeServer {
    public:
        /// Constructor that initialize instance of LogicService
        /// \param server_address listening ip:port
        explicit GrpcNodeServer(const std::string &server_address) : _service{}, _server_address(server_address) {
            _service.init();
        }

        /// plain destructor that call GrpcNodeServer::stop()
        ~GrpcNodeServer(){
            stop();
        }

        /// Register LogicService and start listening port defined in constructor
        void run() {
            grpc::ServerBuilder builder;
            builder.AddListeningPort(_server_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&_service);
            _server = builder.BuildAndStart();
        }

        /// stop listening port defined in constructor and started by GrpcNodeServer::run()
        void stop() {
            _server->Shutdown();
        }

    private:
        const std::string _server_address;
        std::unique_ptr<grpc::Server> _server = nullptr;
        GrpcNodeServiceImpl<LogicService> _service;
    };

}