#include "http_server.hpp"

namespace rpc::http
{

/// Constructor that initialize instance of LogicService
/// \param server_address listening ip:port
NodeServer::NodeServer(const std::string& server_address, std::shared_ptr<BaseRpc> service)
  : _listener(server_address)
{
    _service.init(std::move(service));
    _listener.support(web::http::methods::POST, std::bind(&Adapter::handle_post, &_service, std::placeholders::_1));
    _listener.support(web::http::methods::GET, std::bind(&Adapter::handle_get, &_service, std::placeholders::_1));
}

/// plain destructor that call GrpcNodeServer::stop()
NodeServer::~NodeServer() {}

/// Register LogicService and start listening port defined in constructor
void NodeServer::run()
{
    _listener.open().wait();
}

/// stop listening port defined in constructor and started by GrpcNodeServer::run()
void NodeServer::stop()
{
    _listener.close().wait();
}

}