#pragma once

#include "base_rpc.hpp"

#include <base/property_tree.hpp>

namespace rpc
{


class RpcServer : public BaseRpcServer
{
  public:
    RpcServer(const base::PropertyTree& config, std::shared_ptr<BaseRpc> interface);

    ~RpcServer();

    void run() override;

    void stop() override;

  private:
    std::unique_ptr<BaseRpcServer> _server;
};


class RpcClient: public BaseRpc
{
    enum class Mode
    {
        GRPC = 0,
        HTTP = 1
    };

    RpcClient();

    ~RpcClient();
};

} // namespace rpc
