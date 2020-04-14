#include "http_adapter.hpp"

#include "cpprest/asyncrt_utils.h"
#include "cpprest/filestream.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"

#include <iostream>
#include <string>

namespace rpc::http
{

void Adapter::init(std::shared_ptr<BaseRpc> service)
{
    _service = service;
}

void Adapter::handle_get(web::http::http_request message)
{
    ucout << message.to_string() << std::endl;

    auto paths = web::http::uri::split_path(web::http::uri::decode(message.relative_uri().path()));

    message.relative_uri().path();

    concurrency::streams::fstream::open_istream(U("index.html"), std::ios::in)
      .then([=](concurrency::streams::istream is) {
          message.reply(web::http::status_codes::OK, is, U("text/html")).then([](pplx::task<void> t) {
              try {
                  t.get();
              }
              catch (...) {
                  //
              }
          });
      })
      .then([=](pplx::task<void> t) {
          try {
              t.get();
          }
          catch (...) {
              message.reply(web::http::status_codes::InternalError, U("INTERNAL ERROR "));
          }
      });

    return;
}


void Adapter::handle_post(web::http::http_request message)
{
    ucout << message.to_string() << std::endl;

    message.reply(web::http::status_codes::OK, message.to_string());
    return;
}

}