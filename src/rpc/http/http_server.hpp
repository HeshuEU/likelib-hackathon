#pragma once

#include "http_adapter.hpp"

#include <rpc/base_rpc.hpp>

namespace rpc {
    namespace http {

/// Template server was implemented logic to start listening messages by gRPC
        class Server {
        public:
            /// Constructor that initialize instance of LogicService
            /// \param server_address listening ip:port
            explicit Server(const std::string &server_address, std::shared_ptr<BaseRpc> service);

            /// plain destructor that call GrpcNodeServer::stop()
            ~Server();

            /// Register LogicService and start listening port defined in constructor
            void run();

            /// stop listening port defined in constructor and started by GrpcNodeServer::run()
            void stop();

        private:
            Adapter _service;
            const std::string _server_address;
        };
    }

} // namespace rpc
