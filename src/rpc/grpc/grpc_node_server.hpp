#pragma once

#include "grpc_adapter.hpp"

namespace rpc
{

/// Template server was implemented logic to start listening messages by gRPC
class GrpcNodeServer
{
  public:
    /// Constructor that initialize instance of LogicService
    /// \param server_address listening ip:port
    explicit GrpcNodeServer(const std::string& server_address, std::shared_ptr<BaseRpc> service);

    /// plain destructor that call GrpcNodeServer::stop()
    ~GrpcNodeServer();

    /// Register LogicService and start listening port defined in constructor
    void run();

    /// stop listening port defined in constructor and started by GrpcNodeServer::run()
    void stop();

  private:
    GrpcAdapter _service;
    const std::string _server_address;
    std::unique_ptr<grpc::Server> _server = nullptr;
};

} // namespace rpc
