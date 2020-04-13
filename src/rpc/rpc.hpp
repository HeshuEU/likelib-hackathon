#pragma once

#include "rpc/grpc/grpc_client.hpp"
#include "rpc/grpc/grpc_server.hpp"
#include "rpc/http/http_adapter.hpp"

#include <base/property_tree.hpp>

namespace rpc {

    class RpcServer {

        RpcServer(const base::PropertyTree &config, BaseRpc &interface);

        ~RpcServer();

        void run();

    };


    class RpcClient {
        enum class Mode{
            GRPC = 0,
            HTTP = 1
        };

        RpcClient();

        ~RpcClient();
    };

} // namespace rpc
