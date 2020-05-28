#pragma once

#include "rpc/base_rpc.hpp"

#include "base/property_tree.hpp"

namespace rpc
{

enum class ClientMode
{
    GRPC = 0,
    HTTP = 1
};


std::unique_ptr<BaseRpc> createRpcClient(ClientMode mode, const std::string& connect_address);


class BaseRpcServer
{
  public:
    virtual ~BaseRpcServer() = default;

    virtual void run() = 0;

    virtual void stop() = 0;
};


std::unique_ptr<BaseRpcServer> create_rpc_server(const base::PropertyTree& config, std::shared_ptr<BaseRpc> interface);

} // namespace rpc
