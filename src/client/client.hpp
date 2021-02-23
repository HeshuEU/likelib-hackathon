#pragma once

#include "core/types.hpp"

#include "websocket/client.hpp"
#include "websocket/tools.hpp"

#include <boost/thread.hpp>

class Client
{
  public:
    explicit Client();
    void run();
    void output(const std::string& str);
    void remoteOutput(const std::string& str);
    const std::map<std::string, std::string>& getWallets() const;

  private:
    std::string _prompt;
    std::string _saved_line;
    std::uint32_t _saved_point;

    std::mutex _out_mutex;

    bool _exit{ false };

    boost::asio::io_context _io_context;
    websocket::WebSocketClient _web_socket_client;

    bool _connected{ false };
    std::string _host;

    std::thread _thread;

    std::map<std::string, std::string> _wallets;

    void processLine(std::string line);

    void reactivateReadline();
    void deactivateReadline();

    void chooseAction(std::string& input);

    void printReceivedData(websocket::Command::Id command_id, base::json::Value received_message);

    static char** characterNameCompletion(const char* text, int start, int end);
    static char* characterNameGenerator(const char* text, int state);
};