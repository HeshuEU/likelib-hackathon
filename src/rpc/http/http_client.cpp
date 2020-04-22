#include "http_client.hpp"

#include <rpc/error.hpp>

namespace rpc::http
{

NodeClient::NodeClient(const std::string& connect_address)
  : _client{ U("http://") + U(connect_address) }
{}

lk::AccountInfo NodeClient::getAccount(const lk::Address& address) {}

Info NodeClient::getNodeInfo() {}

lk::Block NodeClient::getBlock(const base::Sha256& block_hash) {}

lk::Transaction NodeClient::getTransaction(const base::Sha256& transaction_hash) {}

TransactionStatus NodeClient::pushTransaction(const lk::Transaction& transaction) {}

TransactionStatus NodeClient::getTransactionResult(const base::Sha256& transaction_hash) {}


// uint32_t NodeClient::get_api_version()
//{
//    web::http::uri_builder builder(U("/get_api_version"));
//    uint32_t result = 0;
//    _client.request(web::http::methods::GET, builder.to_string())
//      .then([&result](web::http::http_response response) {
//          response.extract_json()
//            .then([&result](web::json::value request_body) {
//                if (request_body.at("status").as_string() == "ok") {
//                    auto result_call_object = request_body.at("result");
//                    result = result_call_object.as_object().at("api_version").as_number().to_uint32();
//                }
//            })
//            .wait();
//      })
//      .wait();
//    if (result != 0) {
//        return result;
//    }
//    RAISE_ERROR(rpc::RpcError, "error in call");
//}

} // namespace rpc
