#include "http_adapter.h"

#include <iostream>

std::unique_ptr<handler> g_httpHandler;

void on_initialize(const utility::string_t &address) {
    web::http::uri_builder uri(address);

    auto addr = uri.to_uri().to_string();
    g_httpHandler = std::make_unique<handler>(addr);
    g_httpHandler->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_shutdown() {
    g_httpHandler->close().wait();
    return;
}

int main(int argc, char *argv[]) {
    utility::string_t port = U("34568");
    if (argc == 2) {
        port = argv[1];
    }

    utility::string_t address = U("http://127.0.0.1:");
    address.append(port);

    on_initialize(address);
    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);

    on_shutdown();
    return 0;
}