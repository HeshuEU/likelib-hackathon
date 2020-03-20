#include "grpc_node_server.hpp"

#include "base/error.hpp"
#include "rpc/error.hpp"

namespace rpc
{

GrpcNodeServer::GrpcNodeServer(const std::string& server_address, std::shared_ptr<BaseRpc> service)
  : _service()
  , _server_address(server_address)
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
    int selected_port = -1;
    builder.AddListeningPort(_server_address, channel_credentials, &selected_port);
    builder.RegisterService(&_service);
    _server = builder.BuildAndStart();
    if (selected_port == -1 || selected_port == 0) {
        // _server->Shutdown();
        RAISE_ERROR(rpc::RpcError, "RPC cannot bind to a selected port");
    }
}

void GrpcNodeServer::stop()
{
    _server->Shutdown();
}

} // namespace rpc