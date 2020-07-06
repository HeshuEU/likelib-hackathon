#include "http_client.hpp"

#include "rpc/error.hpp"
#include "rpc/http/tools.hpp"

#include <optional>

namespace
{

web::http::http_request createPostRequest(const std::string& url, const web::json::value& request)
{
    web::http::uri_builder builder(url);

    web::http::http_request request_post;
    request_post.set_method(web::http::methods::POST);
    request_post.set_request_uri(builder.to_uri());
    request_post.set_body(request);

    return request_post;
}

}


namespace rpc::http
{

NodeClient::NodeClient(const std::string& connect_address)
  : _client{ "http://" + U(connect_address) }
{}


lk::AccountInfo NodeClient::getAccountInfo(const lk::Address& address)
{
    web::json::value request_body;
    request_body["address"] = web::json::value::string(base::base58Encode(address.getBytes()));

    std::optional<lk::AccountInfo> opt_account_info;

    _client.request(createPostRequest("/get_account", request_body))
      .then([&](const web::http::http_response& response) {
          response.extract_json()
            .then([&](web::json::value request_body) {
                if (request_body.at("status").as_string() == "ok") {
                    opt_account_info = deserializeAccountInfo(request_body.at("result"));
                }
                else {
                    if (request_body.has_field("result")) {
                        LOG_ERROR << "bad request result:" << request_body.at("result").serialize();
                    }
                    else {
                        LOG_ERROR << "bad request result";
                    }
                    RAISE_ERROR(RpcError, "bad result status");
                }
            })
            .wait();
      })
      .wait();
    if (opt_account_info) {
        return opt_account_info.value();
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "deserialization error");
    }
}


Info NodeClient::getNodeInfo()
{
    web::json::value request_body;

    std::optional<Info> opt_info;

    _client.request(createPostRequest("/get_node_info", request_body))
      .then([&](const web::http::http_response& response) {
          response.extract_json()
            .then([&](web::json::value request_body) {
                if (request_body.at("status").as_string() == "ok") {
                    opt_info = deserializeInfo(request_body.at("result"));
                }
                else {
                    if (request_body.has_field("result")) {
                        LOG_ERROR << "bad request result:" << request_body.at("result").serialize();
                    }
                    else {
                        LOG_ERROR << "bad request result";
                    }
                    RAISE_ERROR(RpcError, "bad result status");
                }
            })
            .wait();
      })
      .wait();
    if (opt_info) {
        return opt_info.value();
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "deserialization error");
    }
}


lk::ImmutableBlock NodeClient::getBlock(const base::Sha256& block_hash)
{
    web::json::value request_body;
    request_body["hash"] = serializeHash(block_hash);

    std::optional<lk::ImmutableBlock> opt_block;

    _client.request(createPostRequest("/get_block", request_body))
      .then([&](const web::http::http_response& response) {
          response.extract_json()
            .then([&](web::json::value request_body) {
                if (request_body.at("status").as_string() == "ok") {
                    auto r = deserializeBlock(request_body.at("result"));
                    if (r) {
                        opt_block.emplace(*r);
                    }
                }
                else {
                    if (request_body.has_field("result")) {
                        LOG_ERROR << "bad request result:" << request_body.at("result").serialize();
                    }
                    else {
                        LOG_ERROR << "bad request result";
                    }
                    RAISE_ERROR(RpcError, "bad result status");
                }
            })
            .wait();
      })
      .wait();
    if (opt_block) {
        return opt_block.value();
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "deserialization error");
    }
}


lk::ImmutableBlock NodeClient::getBlock(uint64_t block_number)
{
    web::json::value request_body;
    request_body["number"] = web::json::value::number(block_number);

    std::optional<lk::ImmutableBlock> opt_block;

    _client.request(createPostRequest("/get_block", request_body))
      .then([&](const web::http::http_response& response) {
          response.extract_json()
            .then([&](web::json::value request_body) {
                if (request_body.at("status").as_string() == "ok") {
                    auto r = deserializeBlock(request_body.at("result"));
                    if (r) {
                        opt_block.emplace(*r);
                    }
                }
                else {
                    if (request_body.has_field("result")) {
                        LOG_ERROR << "bad request result:" << request_body.at("result").serialize();
                    }
                    else {
                        LOG_ERROR << "bad request result";
                    }
                    RAISE_ERROR(RpcError, "bad result status");
                }
            })
            .wait();
      })
      .wait();
    if (opt_block) {
        return opt_block.value();
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "deserialization error");
    }
}


lk::Transaction NodeClient::getTransaction(const base::Sha256& transaction_hash)
{
    web::json::value request_body;
    request_body[U("hash")] = serializeHash(transaction_hash);

    std::optional<lk::Transaction> opt_tx;

    _client.request(createPostRequest("/get_transaction", request_body))
      .then([&](const web::http::http_response& response) {
          response.extract_json()
            .then([&](web::json::value request_body) {
                if (request_body.at("status").as_string() == "ok") {
                    opt_tx = deserializeTransaction(request_body.at("result"));
                }
                else {
                    if (request_body.has_field("result")) {
                        LOG_ERROR << "bad request result:" << request_body.at("result").serialize();
                    }
                    else {
                        LOG_ERROR << "bad request result";
                    }
                    RAISE_ERROR(RpcError, "bad result status");
                }
            })
            .wait();
      })
      .wait();
    if (opt_tx) {
        return opt_tx.value();
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "deserialization error");
    }
}


lk::TransactionStatus NodeClient::pushTransaction(const lk::Transaction& transaction)
{
    web::json::value request_body = serializeTransaction(transaction);

    std::optional<lk::TransactionStatus> opt_tx_status;

    _client.request(createPostRequest("/push_transaction", request_body))
      .then([&](const web::http::http_response& response) {
          response.extract_json()
            .then([&](web::json::value request_body) {
                if (request_body.at("status").as_string() == "ok") {
                    opt_tx_status = deserializeTransactionStatus(request_body.at("result"));
                }
                else {
                    if (request_body.has_field("result")) {
                        LOG_ERROR << "bad request result:" << request_body.at("result").serialize();
                    }
                    else {
                        LOG_ERROR << "bad request result";
                    }
                    RAISE_ERROR(RpcError, "bad result status");
                }
            })
            .wait();
      })
      .wait();
    if (opt_tx_status) {
        return opt_tx_status.value();
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "deserialization error");
    }
}


lk::TransactionStatus NodeClient::getTransactionStatus(const base::Sha256& transaction_hash)
{
    web::json::value request_body;
    request_body[U("hash")] = serializeHash(transaction_hash);

    std::optional<lk::TransactionStatus> opt_tx_status;

    _client.request(createPostRequest("/get_transaction_status", request_body))
      .then([&](const web::http::http_response& response) {
          response.extract_json()
            .then([&](web::json::value request_body) {
                if (request_body.at("status").as_string() == "ok") {
                    opt_tx_status = deserializeTransactionStatus(request_body.at("result"));
                }
                else {
                    if (request_body.has_field("result")) {
                        LOG_ERROR << "bad request result:" << request_body.at("result").serialize();
                    }
                    else {
                        LOG_ERROR << "bad request result";
                    }
                    RAISE_ERROR(RpcError, "bad result status");
                }
            })
            .wait();
      })
      .wait();
    if (opt_tx_status) {
        return opt_tx_status.value();
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "deserialization error");
    }
}

} // namespace rpc
