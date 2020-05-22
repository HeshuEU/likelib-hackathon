#include "http_server.hpp"

namespace rpc::http
{

NodeServer::NodeServer(const std::string& server_address, std::shared_ptr<BaseRpc> service)
  : _listener(server_address)
{
    _service.init(std::move(service));
    _listener.support(web::http::methods::POST, std::bind(&Adapter::handler, &_service, std::placeholders::_1));
}


NodeServer::~NodeServer() {}


void NodeServer::run()
{
    _listener.open().wait();
}


void NodeServer::stop()
{
    _listener.close().wait();
}

}