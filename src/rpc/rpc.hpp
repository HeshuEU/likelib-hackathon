#pragma once

#include "grpc/grpc_node_server.hpp"
#include "grpc/grpc_node_client.hpp"

#include "bc/general_server_service.hpp"

namespace rpc {

    using RpcServer = GrpcNodeServer<bc::GeneralServerService>;
    using RpcClient = GrpcNodeClient;

}
