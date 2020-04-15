#include "rpc.hpp"

#include "grpc/grpc_client.hpp"
#include "grpc/grpc_server.hpp"
#include "http/http_client.hpp"
#include "http/http_server.hpp"

namespace rpc
{


std::unique_ptr<BaseRpcServer> create_rpc_server(const base::PropertyTree& config, std::shared_ptr<BaseRpc> interface)
{
    auto mode = config.get<std::string>("rpc.mode");
    if (mode == "grpc") {
        auto address = config.get<std::string>("rpc.address");
        return std::unique_ptr<rpc::BaseRpcServer>(new rpc::grpc::NodeServer(address, std::move(interface)));
    }
    else if (mode == "http") {
        auto address = config.get<std::string>("rpc.address");
        utility::string_t protocol = U("http://");
        protocol.append(address);
        web::http::uri_builder uri(protocol);

        return std::unique_ptr<rpc::BaseRpcServer>(
          new rpc::http::NodeServer(uri.to_uri().to_string(), std::move(interface)));
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "unexpected mode");
    }
}

std::unique_ptr<BaseRpc> create_rpc_client(ClientMode mode, const std::string& connect_address)
{
    switch (mode) {
        case ClientMode::GRPC:
            return std::unique_ptr<BaseRpc>(new grpc::NodeClient(connect_address));
        case ClientMode::HTTP:
            return std::unique_ptr<BaseRpc>(new http::NodeClient(connect_address));
    }
    RAISE_ERROR(base::LogicError, "Unexpected case");
}


}