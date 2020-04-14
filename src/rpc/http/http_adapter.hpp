#pragma once

#include "cpprest/http_listener.h"

#include <rpc/base_rpc.hpp>


namespace rpc::http
{

class Adapter final
{
  public:
    Adapter() = default;

    ~Adapter() = default;

    void init(std::shared_ptr<BaseRpc> service);

    void handle_get(web::http::http_request message);

    void handle_post(web::http::http_request message);

  private:
    std::shared_ptr<BaseRpc> _service;
};

}