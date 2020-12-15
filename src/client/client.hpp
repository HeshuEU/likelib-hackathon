#pragma once

#include "core/types.hpp"

#include "websocket/client.hpp"
#include "websocket/tools.hpp"

#include <cli/cli.h>

#include <boost/thread.hpp>

class Client
{
  public:
    explicit Client();
    void run();

    void reactivateReadline();
    void deactivateReadline();

    static Client* instance();
  private:
    std::string _prompt;
    bool readline_active;
    std::string _saved_line;
    std::uint32_t _saved_point;
    std::vector<std::string> _enter_strings;

    static Client* _instance;

    static void s_line(char* line);
    // std::vector<cli::CmdHandler> _disconnected_mode_commands{};
    // std::vector<cli::CmdHandler> _always_mode_commands{};
    // std::vector<cli::CmdHandler> _connected_mode_commands{};

    // std::unique_ptr<cli::Menu> _root_menu;

    // boost::asio::io_context _io_context;
    // websocket::WebSocketClient _web_socket_client;
    // boost::thread _networkThread;

    // void printReceivedData(websocket::Command::Id command_id, base::json::Value received_message);
    // static void disableCommands(std::vector<cli::CmdHandler>& commands);
    // static void enableCommands(std::vector<cli::CmdHandler>& commands);

    // void setupCli();
};