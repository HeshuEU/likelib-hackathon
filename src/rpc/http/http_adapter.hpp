#pragma once

#include "cpprest/http_listener.h"


namespace rpc {

    namespace http {

        class Adapter {
        public:
            explicit Adapter(utility::string_t url);

            virtual ~Adapter();

            pplx::task<void> open() { return m_listener.open(); }

            pplx::task<void> close() { return m_listener.close(); }

        private:
            void handle_get(web::http::http_request message);

            void handle_post(web::http::http_request message);

            web::http::experimental::listener::http_listener m_listener;
        };

    }
}