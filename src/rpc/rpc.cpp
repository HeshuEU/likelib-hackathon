#include "rpc.hpp"

#include "rpc/grpc/grpc_client.hpp"
#include "rpc/grpc/grpc_server.hpp"
#include "rpc/http/http_client.hpp"
#include "rpc/http/http_server.hpp"

namespace rpc
{

namespace detail
{

class RpcServer : public BaseRpcServer
{
  public:
    RpcServer(const base::PropertyTree& config, std::shared_ptr<BaseRpc> interface);

    ~RpcServer() override;

    void run() override;

    void stop() override;

  private:
    static constexpr const std::size_t GRPC = 0x1;
    static constexpr const std::size_t HTTP = 0x2;

    std::size_t _mode = 0x0;

    std::unique_ptr<BaseRpcServer> _grpc_server;
    std::string _grpc_listening_address;

    std::unique_ptr<BaseRpcServer> _http_server;
    std::string _http_listening_address;
};


RpcServer::~RpcServer()
{
    stop();
}

RpcServer::RpcServer(const base::PropertyTree& config, std::shared_ptr<BaseRpc> interface)
{
    if (config.hasKey("rpc.grpc_address")) {
        _grpc_listening_address = config.get<std::string>("rpc.grpc_address");
        _grpc_server = std::make_unique<rpc::grpc::NodeServer>(_grpc_listening_address, interface);
        _mode = _mode | GRPC;
    }
    if (config.hasKey("rpc.http_address")) {
        _http_listening_address = "http://" + config.get<std::string>("rpc.http_address");
        _http_server = std::make_unique<rpc::http::NodeServer>(_http_listening_address, interface);
        _mode = _mode | HTTP;
    }
    if (!_mode) {
        RAISE_ERROR(base::InvalidArgument, "RPC server was not chosen");
    }
}

void RpcServer::run()
{
    if (_mode & HTTP) {
        _http_server->run();
        LOG_INFO << "HTTP RPC server started: " << _http_listening_address;
    }
    if (_mode & GRPC) {
        _grpc_server->run();
        LOG_INFO << "GRPC RPC server started: " << _grpc_listening_address;
    }
}

void RpcServer::stop()
{
    if (_mode & HTTP) {
        _http_server->stop();
        LOG_INFO << "HTTP RPC server was stopped";
    }
    if (_mode & GRPC) {
        _grpc_server->stop();
        LOG_INFO << "GRPC RPC server was stopped";
    }
}

}


std::unique_ptr<BaseRpc> createRpcClient(ClientMode mode, const std::string& connect_address)
{
    switch (mode) {
        case ClientMode::GRPC:
            return std::unique_ptr<BaseRpc>(new grpc::NodeClient(connect_address));
        case ClientMode::HTTP:
            return std::unique_ptr<BaseRpc>(new http::NodeClient(connect_address));
        default:
            RAISE_ERROR(base::LogicError, "Unexpected case");
    }
}


std::unique_ptr<BaseRpcServer> create_rpc_server(const base::PropertyTree& config, std::shared_ptr<BaseRpc> interface)
{
    return std::make_unique<detail::RpcServer>(config, interface);
}

}