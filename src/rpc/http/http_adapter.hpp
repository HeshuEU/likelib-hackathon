#pragma once

#include "rpc/base_rpc.hpp"

#include <cpprest/http_listener.h>

namespace rpc::http
{

class Adapter final
{
  public:
    Adapter() = default;

    ~Adapter() = default;

    void init(std::shared_ptr<BaseRpc> service);

    void handler(const web::http::http_request& message);

  private:
    std::shared_ptr<BaseRpc> _service;

    using JsonProcessorFn =
      std::function<web::json::value(const web::http::http_request& message, std::shared_ptr<rpc::BaseRpc>&)>;
    std::map<std::string, JsonProcessorFn> _json_processors;

    using EmptyProcessorFn = std::function<web::json::value(std::shared_ptr<rpc::BaseRpc>&)>;
    std::map<std::string, EmptyProcessorFn> _empty_processors;
};

}