#include "http_server.hpp"

namespace rpc {
    namespace http {

/// Constructor that initialize instance of LogicService
/// \param server_address listening ip:port
        explicit Server::Server(const std::string &server_address, std::shared_ptr<BaseRpc> service) {}

/// plain destructor that call GrpcNodeServer::stop()
        Server::~Server() {}

/// Register LogicService and start listening port defined in constructor
        void Server::run() {
            utility::string_t port = U("34568");
            utility::string_t address = U("http://127.0.0.1:");
            address.append(port);

            web::http::uri_builder uri(address);

            auto addr = uri.to_uri().to_string();
            _service.open().wait();
        }

        /// stop listening port defined in constructor and started by GrpcNodeServer::run()
        void Server::stop() {
            _service.close().wait();
        }

    }
}