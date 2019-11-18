#pragma once

#include "grpc_service.hpp"

namespace rpc
{

/// Template server was implemented logic to start listening messages by gRPC
/// \tparam LogicService blockchain logic interface class implemented from bc::BaseService
template<typename LogicService>
class GrpcNodeServer
{
  public:
    /// Constructor that initialize instance of LogicService
    /// \param server_address listening ip:port
    explicit GrpcNodeServer(const std::string& server_address);

    /// plain destructor that call GrpcNodeServer::stop()
    ~GrpcNodeServer();

    /// Register LogicService and start listening port defined in constructor
    void run();

    /// stop listening port defined in constructor and started by GrpcNodeServer::run()
    void stop();

  private:
    const std::string _server_address;
    std::unique_ptr<grpc::Server> _server = nullptr;
    GrpcNodeServiceImpl<LogicService> _service;
};

} // namespace rpc

#include "grpc_node_server.tpp"
