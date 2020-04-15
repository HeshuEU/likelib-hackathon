#include "http_adapter.hpp"

#include "cpprest/filestream.h"
#include <cpprest/asyncrt_utils.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>

#include <iostream>
#include <string>


namespace detail
{

class ActionBase
{
  public:
    //====================================
    explicit ActionBase(std::shared_ptr<rpc::BaseRpc>& service);
    virtual ~ActionBase() = default;
    //====================================
    virtual const std::string& getName() const = 0;
    virtual void run(web::json::value& result) = 0;
    //====================================
  protected:
    std::shared_ptr<rpc::BaseRpc>& _service;
};


ActionBase::ActionBase(std::shared_ptr<rpc::BaseRpc>& service)
  : _service{ service }
{}


class ActionInfo : public ActionBase
{
  public:
    //====================================
    explicit ActionInfo(std::shared_ptr<rpc::BaseRpc>& service);
    virtual ~ActionInfo() = default;
    //====================================
    const std::string& getName() const override;
    void run(web::json::value& result) override;
};


ActionInfo::ActionInfo(std::shared_ptr<rpc::BaseRpc>& service)
  : ActionBase(service)
{}


const std::string& ActionInfo::getName() const
{
    static const std::string name = "info";
    return name;
}


void ActionInfo::run(web::json::value& result)
{
    auto top_block_hash = _service->info().top_block_hash.toHex();
    result[U("top_block_hash")] = web::json::value::string(top_block_hash);
}


class ActionGetApiVersion : public ActionBase
{
  public:
    //====================================
    explicit ActionGetApiVersion(std::shared_ptr<rpc::BaseRpc>& service);
    virtual ~ActionGetApiVersion() = default;
    //====================================
    const std::string& getName() const override;
    void run(web::json::value& result) override;

  private:
};


ActionGetApiVersion::ActionGetApiVersion(std::shared_ptr<rpc::BaseRpc>& service)
  : ActionBase(service)
{}


const std::string& ActionGetApiVersion::getName() const
{
    static const std::string name = "get_api_version";
    return name;
}

void ActionGetApiVersion::run(web::json::value& result)
{
    auto version = _service->get_api_version();
    result[U("api_version")] = web::json::value::number(version);
}


class ActionPostBase : public ActionBase
{
  public:
    //====================================
    explicit ActionPostBase(web::json::value& input, std::shared_ptr<rpc::BaseRpc>& service);
    virtual ~ActionPostBase() = default;
    //====================================
  protected:
    web::json::value _input;
};


ActionPostBase::ActionPostBase(web::json::value& input, std::shared_ptr<rpc::BaseRpc>& service)
  : ActionBase(service)
  , _input{ input }
{}


class ActionGetBalance : public ActionPostBase
{
  public:
    //====================================
    explicit ActionGetBalance(web::json::value& input, std::shared_ptr<rpc::BaseRpc>& service);
    virtual ~ActionGetBalance() = default;
    //====================================
    const std::string& getName() const override;
    void run(web::json::value& result) override;

  private:
};


ActionGetBalance::ActionGetBalance(web::json::value& input, std::shared_ptr<rpc::BaseRpc>& service)
  : ActionPostBase(input, service)
{}


const std::string& ActionGetBalance::getName() const
{
    static const std::string name = "get_balance";
    return name;
}


void ActionGetBalance::run(web::json::value& result)
{
    auto block_hash_value = _input.at(U("address"));
    auto address = lk::Address{ block_hash_value.as_string() };
    auto balance = _service->balance(address);
    result[U("address")] = block_hash_value;
    result[U("balance")] = web::json::value::number(balance);
}


class ActionGetBlock : public ActionPostBase
{
  public:
    //====================================
    explicit ActionGetBlock(web::json::value& input, std::shared_ptr<rpc::BaseRpc>& service);
    virtual ~ActionGetBlock() = default;
    //====================================
    const std::string& getName() const override;
    void run(web::json::value& result) override;

  private:
};


ActionGetBlock::ActionGetBlock(web::json::value& input, std::shared_ptr<rpc::BaseRpc>& service)
  : ActionPostBase(input, service)
{}


const std::string& ActionGetBlock::getName() const
{
    static const std::string name = "get_block";
    return name;
}


void ActionGetBlock::run(web::json::value& result)
{
    auto block_hash_value = _input.at(U("block_hash"));
    auto block_hash = base::Sha256{ base::fromHex<base::Bytes>(block_hash_value.as_string()) };
    auto block = _service->get_block(block_hash);

    web::json::value block_value;
    block_value[U("block_depth")] = web::json::value::number(block.getDepth());
    block_value[U("nonce")] = web::json::value::number(block.getNonce());
    block_value[U("coinbase")] = web::json::value::string(block.getCoinbase().toString());
    block_value[U("previous_block_hash")] = web::json::value::string(block.getPrevBlockHash().toHex());
    block_value[U("timestamp")] = web::json::value::number(block.getTimestamp().getSecondsSinceEpoch());

    std::vector<web::json::value> txs_values;
    for (auto& tx : block.getTransactions()) {
        web::json::value tx_value;
        tx_value[U("from")] = web::json::value::string(tx.getFrom().toString());
        tx_value[U("to")] = web::json::value::string(tx.getTo().toString());
        tx_value[U("amount")] = web::json::value::number(tx.getAmount());
        tx_value[U("fee")] = web::json::value::number(tx.getFee());
        tx_value[U("timestamp")] = web::json::value::number(tx.getTimestamp().getSecondsSinceEpoch());
        tx_value[U("type")] = web::json::value::number(static_cast<uint32_t>(tx.getType()));
        tx_value[U("data")] = web::json::value::string(base::toHex(tx.getData()));
        tx_value[U("sign")] =
          web::json::value::string(tx.getSign().toBase64()); // TODO: think about cross-language serialization
        txs_values.emplace_back(tx_value);
    }
    block_value[U("transactions")] = web::json::value::array(txs_values);

    result[U("block_hash")] = block_hash_value;
    result[U("block")] = block_value;
}

template<typename T>
web::json::value run_get(std::shared_ptr<rpc::BaseRpc>& service)
{
    T action(service);
    web::json::value result;
    result[U("method")] = web::json::value::string(action.getName());
    try {
        web::json::value action_result;
        action.run(action_result);
        result[U("status")] = web::json::value::string("ok");
        result[U("result")] = action_result;
    }
    catch (const std::exception& e) {
        result[U("status")] = web::json::value::string("error");
    }
    return result;
}

template<typename T>
web::json::value run_post(web::json::value& json_body, std::shared_ptr<rpc::BaseRpc>& service)
{
    T action(json_body, service);
    web::json::value result;
    result[U("method")] = web::json::value::string(action.getName());
    try {
        web::json::value action_result;
        action.run(action_result);
        result[U("status")] = web::json::value::string("ok");
        result[U("result")] = action_result;
    }
    catch (const std::exception& e) {
        result[U("status")] = web::json::value::string("error");
    }
    return result;
}

}

namespace rpc::http
{

void Adapter::init(std::shared_ptr<BaseRpc> service)
{
    _service = std::move(service);

    _get_processors.insert({ "info", detail::run_get<detail::ActionInfo> });
    _get_processors.insert({ "get_api_version", detail::run_get<detail::ActionGetApiVersion> });

    _post_processors.insert({ "get_balance", detail::run_post<detail::ActionGetBalance> });
    _post_processors.insert({ "get_block", detail::run_post<detail::ActionGetBlock> });
}

void Adapter::handle_post(const web::http::http_request& message)
{
    auto paths = web::http::uri::split_path(web::http::uri::decode(message.relative_uri().path()));

    if (paths.empty()) {
        RAISE_ERROR(base::InvalidArgument, "inaccessible path");
    }

    auto root_path = paths[0];
    web::json::value request_json;

    message.extract_json()
      .then([&request_json](web::json::value request_body) { request_json = std::move(request_body); })
      .wait();

    if (auto it = _post_processors.find(root_path); it != _post_processors.end()) {
        auto reply_json = it->second(request_json, _service);
        message.reply(web::http::status_codes::OK, reply_json);
        return;
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "processor was not found");
    }
}

void Adapter::handle_get(const web::http::http_request& message)
{
    auto paths = web::http::uri::split_path(web::http::uri::decode(message.relative_uri().path()));

    if (paths.empty()) {
        RAISE_ERROR(base::InvalidArgument, "inaccessible path");
    }

    auto root_path = paths[0];

    if (auto it = _get_processors.find(root_path); it != _get_processors.end()) {
        auto reply_json = it->second(_service);
        message.reply(web::http::status_codes::OK, reply_json);
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "processor was not found");
    }
}

}