#pragma once

#include <rpc/base_rpc.hpp>

#include <cpprest/http_listener.h>

namespace rpc::http
{

class Adapter final
{
  public:
    Adapter() = default;

    ~Adapter() = default;

    void init(std::shared_ptr<BaseRpc> service);

    void handle_post(const web::http::http_request& message);
    void handle_get(const web::http::http_request& message);

  private:
    std::shared_ptr<BaseRpc> _service;
    std::map<std::string,
             std::function<web::json::value(web::json::value& json_body, std::shared_ptr<rpc::BaseRpc>&)>>
      _post_processors;
    std::map<std::string, std::function<web::json::value(std::shared_ptr<rpc::BaseRpc>&)>> _get_processors;
};

}