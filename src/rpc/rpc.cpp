#include "rpc.hpp"

#include "rpc/grpc/grpc_server.hpp"
#include "rpc/http/http_server.hpp"

namespace rpc
{

RpcServer::RpcServer(const base::PropertyTree& config, std::shared_ptr<BaseRpc> interface)
{
    auto mode = config.get<std::string>("rpc.mode");
    if (mode == "grpc") {
        auto address = config.get<std::string>("rpc.address");
        _server = std::unique_ptr<rpc::BaseRpcServer>(new rpc::grpc::NodeServer(address, std::move(interface)));
    }
    else if (mode == "http") {
        auto address = config.get<std::string>("rpc.address");
        utility::string_t protocol = U("http://");
        protocol.append(address);
        web::http::uri_builder uri(protocol);

        _server = std::unique_ptr<rpc::BaseRpcServer>(
          new rpc::http::NodeServer(uri.to_uri().to_string(), std::move(interface)));
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "unexpected mode");
    }
}

RpcServer::~RpcServer()
{
    (*this).stop();
}

void RpcServer::run()
{
    _server->run();
}

void RpcServer::stop()
{
    _server->stop();
}

}