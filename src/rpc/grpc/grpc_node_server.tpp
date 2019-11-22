#pragma once
#include "grpc_node_server.hpp"

namespace rpc
{

template<typename LogicService>
GrpcNodeServer<LogicService>::GrpcNodeServer(const std::string& server_address)
    : _service(), _server_address(server_address)
{
    _service.init();
}

template<typename LogicService>
GrpcNodeServer<LogicService>::~GrpcNodeServer()
{
    stop();
}

template<typename LogicService>
void GrpcNodeServer<LogicService>::run()
{
    grpc::ServerBuilder builder;
    auto channel_credentials = grpc::InsecureServerCredentials();
    builder.AddListeningPort(_server_address, channel_credentials);
    builder.RegisterService(&_service);
    _server = builder.BuildAndStart();
}

template<typename LogicService>
void GrpcNodeServer<LogicService>::stop()
{
    _server->Shutdown();
}

} // namespace rpc