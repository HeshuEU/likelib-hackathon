#pragma once

#include "rpc/http/http_adapter.hpp"

#include "rpc/rpc.hpp"

namespace rpc::http
{

/// Template server was implemented logic to start listening messages by gRPC
class NodeServer final : public BaseRpcServer
{
  public:
    /// Constructor that initialize instance of LogicService
    /// \param server_address listening ip:port
    NodeServer(const std::string& server_address, std::shared_ptr<BaseRpc> service);

    /// plain destructor that call GrpcNodeServer::stop()
    ~NodeServer() override;

    /// Register LogicService and start listening port defined in constructor
    void run() override;

    /// stop listening port defined in constructor and started by GrpcNodeServer::run()
    void stop() override;

  private:
    web::http::experimental::listener::http_listener _listener;
    Adapter _service;
};

} // namespace rpc
