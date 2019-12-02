#include "grpc_node_server.hpp"

namespace rpc
{

GrpcNodeServer::GrpcNodeServer(const std::string& server_address, std::shared_ptr<BaseRpc> service)
    : _service(), _server_address(server_address)
{
    _service.init(service);
}

GrpcNodeServer::~GrpcNodeServer()
{
    stop();
}

void GrpcNodeServer::run()
{
    grpc::ServerBuilder builder;
    auto channel_credentials = grpc::InsecureServerCredentials();
    builder.AddListeningPort(_server_address, channel_credentials);
    builder.RegisterService(&_service);
    _server = builder.BuildAndStart();
}

void GrpcNodeServer::stop()
{
    _server->Shutdown();
}

} // namespace rpc