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

  private:
    std::vector<cli::CmdHandler> _disconnected_mode_commands{};
    std::vector<cli::CmdHandler> _always_mode_commands{};
    std::vector<cli::CmdHandler> _connected_mode_commands{};

    std::unique_ptr<cli::Menu> _root_menu;

    boost::asio::io_context _io_context;
    websocket::WebSocketClient _web_socket_client;
    boost::thread _networkThread;

    void printReceivedData(websocket::Command::Id command_id, base::PropertyTree received_message);
    static void disableCommands(std::vector<cli::CmdHandler>& commands);
    static void enableCommands(std::vector<cli::CmdHandler>& commands);

    void setupCli();
};