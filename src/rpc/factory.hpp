#pragma once

#include "rpc/grpc/grpc_node_server.hpp"

#include <memory>

namespace rpc {

    template<typename Service>
    static std::unique_ptr<GrpcNodeServer<Service>> createAndStartGrpcServerInstance(const std::string &server_address) {
        std::unique_ptr<GrpcNodeServer<Service>> ptr = std::make_unique<GrpcNodeServer<Service>>(server_address);
        ptr->run();
        return std::move(ptr);
    }

}