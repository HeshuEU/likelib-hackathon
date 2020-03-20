#pragma once

#include "grpc/grpc_node_client.hpp"
#include "grpc/grpc_node_server.hpp"

namespace rpc
{

using RpcServer = GrpcNodeServer;
using RpcClient = GrpcNodeClient;

} // namespace rpc
