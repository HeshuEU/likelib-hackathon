#include "client.hpp"
#include "actions.hpp"

#include <readline/history.h>
#include <readline/readline.h>

#include <iostream>

void clearSpaces(std::string& str)
{
    std::size_t count_spaces{ 0 };
    while (count_spaces < str.size() && (str[count_spaces] == ' ' || str[count_spaces] == '\t')) {
        count_spaces++;
    }
    if (count_spaces >= str.size()) {
        str.clear();
    }
    else {
        str.erase(0, count_spaces);
    }
}

std::string parseActionName(std::string& input)
{
    clearSpaces(input);
    std::size_t end{ 0 };
    while (end < input.size() && input[end] != ' ') {
        end++;
    }
    auto action_name = input.substr(0, end);
    input.erase(0, end);
    clearSpaces(input);
    return action_name;
}


std::string parseArgument(std::string& input)
{
    clearSpaces(input);
    if (input.empty()) {
        return input;
    }
    char quote;
    bool is_text{ false };
    if ((input[0] == '\'') || (input[0] == '\"')) {
        quote = input[0];
        is_text = true;
        input.erase(0, 1);
    }
    std::size_t end{ 0 };

    while ((end < input.size()) && (is_text || (input[end] != ' '))) {
        if (!is_text && ((input[end] == '\'') || (input[end] == '\"'))) {
            is_text = true;
            quote = input[end];
            input.erase(end, 1);
        }
        else if (!is_text && (input[end] == '\\')) {
            input.erase(end, 1);
            if (input.size() <= end) {
                return std::string{};
            }
            else {
                end++;
            }
        }
        else if (is_text && input[end] == quote) {
            is_text = false;
            input.erase(end, 1);
        }
        else {
            end++;
        }
    }
    if (is_text) {
        return std::string{};
    }
    auto argument = input.substr(0, end);
    input.erase(0, end);
    return argument;
}


std::vector<std::string> parseAllArguments(std::string& input)
{
    std::vector<std::string> arguments;
    while (!input.empty()) {
        auto argument = parseArgument(input);
        if (!argument.empty()) {
            arguments.push_back(argument);
        }
    }
    return arguments;
}

void Client::chooseAction(std::string& input)
{
    auto action_name = parseActionName(input);
    if (action_name.empty()) {
        return;
    }
    const auto& arguments = parseAllArguments(input);

    if (action_name == "help") {
        if (arguments.size() != 0) {
            output("Wrong number of arguments for the help command");
            return;
        }
        help(*this);
    }
    else if (action_name == "connect") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the connect command");
        }
        if (_connected) {
            output("You are already connected to host " + _host + "\n disconnect before new reconnecting");
            return;
        }

        auto host = arguments[0];
        _connected = _web_socket_client.connect(host);
        if (_connected) {
            _host = host;
            output("Client connected to host " + host);
        }
        else {
            output("Can't connect to host " + host);
        }
    }
    else if (action_name == "disconnect") {
        if (arguments.size() != 0) {
            output("Wrong number of arguments for the disconnect command");
            return;
        }
        if (!_connected) {
            output("You aren't connected to the node");
            return;
        }

        _connected = false;
        _web_socket_client.disconnect();
    }
    else if (action_name == "compile") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the compile command");
            return;
        }

        const auto path = arguments[0];
        compile_solidity_code(*this, path);
    }
    else if (action_name == "encode") {
        if (arguments.size() != 2) {
            output("Wrong number of arguments for the encode command");
            return;
        }

        const auto path = arguments[0];
        const auto message = arguments[1];
        encode_message(*this, path, message);
    }
    else if (action_name == "decode") {
        if (arguments.size() != 2) {
            output("Wrong number of arguments for the decode command");
            return;
        }

        const auto path = arguments[0];
        const auto message = arguments[1];
        decode_message(*this, path, message);
    }
    else if (action_name == "keys_generate") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the keys_generate command");
            return;
        }

        const auto path = arguments[0];
        generate_keys(*this, path);
    }
    else if (action_name == "keys_info") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the keys_info command");
            return;
        }

        const auto path = arguments[0];
        keys_info(*this, path);
    }
    else if (action_name == "last_block_info") {
        if (arguments.size() != 0) {
            output("Wrong number of arguments for the last_block_info command");
            return;
        }

        call_last_block_info(_web_socket_client);
    }
    else if (action_name == "account_info") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the account_info command");
            return;
        }

        try {
            const lk::Address address{ arguments[0] };
            call_account_info(_web_socket_client, address);
        }
        catch (const base::Error& e) {
            output("can't execute account_info");
            LOG_ERROR << "can't execute account_info:" << e.what();
        }
    }
    else if (action_name == "subscribe_account_info") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the subscribe_account_info command");
            return;
        }

        try {
            const lk::Address address{ arguments[0] };
            subscribe_account_info(_web_socket_client, address);
        }
        catch (const base::Error& e) {
            output("can't execute subscribe_account_info");
            LOG_ERROR << "can't execute subscribe_account_info:" << e.what();
        }
    }
    else if (action_name == "unsubscribe_account_info") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the unsubscribe_account_info command");
            return;
        }

        try {
            lk::Address address{ arguments[0] };
            unsubscribe_account_info(_web_socket_client, address);
        }
        catch (const base::Error& e) {
            output("can't execute unsubscribe_account_info");
            LOG_ERROR << "can't execute unsubscribe_account_info:" << e.what();
        }
    }
    else if (action_name == "find_transaction") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the find_transaction command");
            return;
        }

        const auto hash = base::Sha256::fromHex(arguments[0]);
        call_find_transaction(_web_socket_client, hash);
    }
    else if (action_name == "find_transaction_status") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the find_transaction_status command");
            return;
        }

        const auto hash = base::Sha256::fromHex(arguments[0]);
        call_find_transaction_status(_web_socket_client, hash);
    }
    else if (action_name == "find_block") {
        if (arguments.size() != 1) {
            output("Wrong number of arguments for the find_block command");
            return;
        }

        const auto hash = base::Sha256::fromHex(arguments[0]);
        call_find_block(_web_socket_client, hash);
    }
    else if (action_name == "transfer") {
        if (arguments.size() != 4) {
            output("Wrong number of arguments for the transfer command");
            return;
        }

        const lk::Address to_address{ arguments[0] };
        const lk::Balance amount{ arguments[1] };
        const lk::Fee fee{ std::stoull(arguments[2]) };
        const std::filesystem::path& keys_dir{ arguments[3] };
        transfer(*this, _web_socket_client, to_address, amount, fee, keys_dir);
    }
    else if (action_name == "contract_call") {
        if (arguments.size() != 5) {
            output("Wrong number of arguments for the contract_call command");
            return;
        }

        const lk::Address to_address{ arguments[0] };
        const lk::Balance amount{ arguments[1] };
        const lk::Fee fee{ std::stoull(arguments[2]) };
        const std::filesystem::path& keys_dir{ arguments[3] };
        const auto message{ arguments[4] };
        contract_call(*this, _web_socket_client, to_address, amount, fee, keys_dir, message);
    }
    else if (action_name == "push_contract") {
        if (arguments.size() != 5) {
            output("Wrong number of arguments for the push_contract command");
            return;
        }

        const lk::Balance amount{ arguments[0] };
        const lk::Fee fee{ std::stoull(arguments[1]) };
        const std::filesystem::path& keys_dir{ arguments[2] };
        const std::filesystem::path& path_to_compiled_folder{ arguments[3] };
        const auto message{ arguments[4] };
        push_contract(*this, _web_socket_client, amount, fee, keys_dir, path_to_compiled_folder, message);
    }
    else if (action_name == "subscribe_last_block_info") {
        if (arguments.size() != 0) {
            output("Wrong number of arguments for the subscribe_last_block_info command");
            return;
        }

        try {
            subscribe_last_block_info(_web_socket_client);
        }
        catch (const base::Error& e) {
            output("can't execute subscribe_last_block_info");
            LOG_ERROR << "can't execute subscribe_last_block_info:" << e.what();
        }
    }
    else if (action_name == "unsubscribe_last_block_info") {
        if (arguments.size() != 0) {
            output("Wrong number of arguments for the unsubscribe_last_block_info command");
            return;
        }

        try {
            unsubscribe_last_block_info(_web_socket_client);
        }
        catch (const base::Error& e) {
            output("can't execute unsubscribe_last_block_info");
            LOG_ERROR << "can't execute unsubscribe_last_block_info:" << e.what();
        }
    }
    else {
        output("There is no command with the name " + action_name);
    }
}


void Client::processLine(std::string line)
{
    clearSpaces(line);
    if (line.empty()) {
        return;
    }
    else {
        add_history(line.c_str());
        chooseAction(line);
    }
}

Client::Client()
  : _prompt("Likelib: ")
  , _io_context{}
  , _web_socket_client{ _io_context,
                        std::bind(&Client::printReceivedData, this, std::placeholders::_1, std::placeholders::_2),
                        [this]() { cli::Cli::cout() << "Disconnected from likelib node\n"; } }
{}


void Client::run()
{
    while (true) {
        const auto line = readline(_prompt.c_str());

        if (line && *line) {
            processLine(line);
        }
        rl_free(line);
    }
}


void Client::output(const std::string& str)
{
    _out_mutex.lock();
    std::cin.clear();
    deactivateReadline();
    std::cout << str << std::endl;
    reactivateReadline();
    _out_mutex.unlock();
}


void Client::deactivateReadline()
{
    _saved_point = rl_point;
    _saved_line = std::string(rl_line_buffer, rl_end);

    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_redisplay();
}


void Client::reactivateReadline()
{
    rl_set_prompt(_prompt.c_str());
    rl_point = _saved_point;
    rl_replace_line(_saved_line.c_str(), 0);
    rl_redisplay();
}


void Client::printReceivedData(websocket::Command::Id command_id, base::json::Value received_message)
{
    output("Received data: " + received_message.serialize() + "\n");
    // TODO
}

// Client::Client()
//   : _root_menu{ std::make_unique<cli::Menu>("empty") }
//   , _io_context{}
//   , _web_socket_client{ _io_context,
//                         std::bind(&Client::printReceivedData, this, std::placeholders::_1,
//                         std::placeholders::_2), [this]() {
//                             cli::Cli::cout() << "Disconnected from likelib node\n";
//                             disableCommands(_connected_mode_commands);
//                             enableCommands(_disconnected_mode_commands);
//                         } }
// {}


// void Client::run()
// {
//     setupCli();
//     cli::Cli cli(std::move(_root_menu));
//     cli::SetColor();

//     cli::CliLocalSession session(cli, _io_context, std::cout, 200);
//     session.ExitAction([this](auto& out) {
//         out << "Closing App...\n";
//         _io_context.stop();
//     });
//     cli::CliTelnetServer server(_io_context, 5000, cli);
//     // exit action for all the connections
//     server.ExitAction([](auto& out) { out << "Terminating this session...\n"; });

//     _io_context.run();
// }


// void Client::printReceivedData(websocket::Command::Id command_id, base::json::Value received_message)
// {
//     cli::Cli::cout() << "Received data: " << received_message.serialize() << "\n";
//     // TODO
// }


// void Client::disableCommands(std::vector<cli::CmdHandler>& commands)
// {
//     for (cli::CmdHandler& command : commands) {
//         command.Disable();
//     }
// }


// void Client::enableCommands(std::vector<cli::CmdHandler>& commands)
// {
//     for (cli::CmdHandler& command : commands) {
//         command.Enable();
//     }
// }


// void Client::setupCli()
// {
//     _disconnected_mode_commands.clear();
//     _always_mode_commands.clear();
//     _connected_mode_commands.clear();

//     _root_menu = std::make_unique<cli::Menu>("likelib");

//     _disconnected_mode_commands.push_back(_root_menu->Insert("connect",
//                                                              [this](std::ostream& out, std::string host) {
//                                                                  if (_web_socket_client.connect(host)) {
//                                                                      out << "Connected to node: " << host <<
//                                                                      "\n";
//                                                                      disableCommands(_disconnected_mode_commands);
//                                                                      enableCommands(_connected_mode_commands);
//                                                                  }
//                                                                  else {
//                                                                      out << "can't connect to node: " << host <<
//                                                                      "\n";
//                                                                  }
//                                                              },
//                                                              "Connect client to specific likelib node",
//                                                              { "ip and port of likelib node" }));

//     _always_mode_commands.push_back(_root_menu->Insert(
//       "compile",
//       [](std::ostream& out, std::string code_file_path) { compile_solidity_code(out, code_file_path); },
//       "compile solidity code to binary format",
//       { "path to solidity file for compilation" }));

//     _always_mode_commands.push_back(
//       _root_menu->Insert("encode",
//                          [](std::ostream& out, std::string compiled_contract_folder_path, std::string message) {
//                              encode_message(out, compiled_contract_folder_path, message);
//                          },
//                          "encode message for contract call",
//                          { "path to compiled contract data files", "message for encode" }));

//     _always_mode_commands.push_back(
//       _root_menu->Insert("decode",
//                          [](std::ostream& out, std::string compiled_contract_folder_path, std::string message) {
//                              decode_message(out, compiled_contract_folder_path, message);
//                          },
//                          "decode message which was returned by contract call",
//                          { "path to compiled contract data files", "message for decode" }));

//     _always_mode_commands.push_back(
//       _root_menu->Insert("keys_generate",
//                          [](std::ostream& out, std::string path) { generate_keys(out, path); },
//                          "generate new key and store to specific folder",
//                          { "path to folder to save key" }));

//     _always_mode_commands.push_back(
//       _root_menu->Insert("keys_info",
//                          [](std::ostream& out, std::string path) { keys_info(out, path); },
//                          "print info about specified key",
//                          { "path to folder with key" }));

//     _connected_mode_commands.push_back(_root_menu->Insert(
//       "last_block_info",
//       [this]([[maybe_unused]] std::ostream& out) { call_last_block_info(_web_socket_client); },
//       "get last block info"));

//     _connected_mode_commands.push_back(_root_menu->Insert(
//       "subscribe_last_block_info",
//       [this]([[maybe_unused]] std::ostream& out) { subscribe_last_block_info(_web_socket_client); },
//       "get last block info when apeared new block"));

//     _connected_mode_commands.push_back(_root_menu->Insert(
//       "unsubscribe_last_block_info",
//       [this]([[maybe_unused]] std::ostream& out) { unsubscribe_last_block_info(_web_socket_client); },
//       "get last block info when apeared new block"));

//     _connected_mode_commands.push_back(_root_menu->Insert("account_info",
//                                                           [this](std::ostream& out, std::string
//                                                           address_at_base58) {
//                                                               try {
//                                                                   lk::Address target_address(address_at_base58);
//                                                                   call_account_info(_web_socket_client,
//                                                                   target_address);
//                                                               }
//                                                               catch (const base::Error& e) {
//                                                                   out << "can't execute account_info";
//                                                                   LOG_ERROR << "can't execute account_info:"
//                                                                             << e.what();
//                                                               }
//                                                           },
//                                                           "get account info by specific address",
//                                                           { "address at base58" }));

//     _connected_mode_commands.push_back(
//       _root_menu->Insert("subscribe_account_info",
//                          [this](std::ostream& out, std::string address_at_base58) {
//                              try {
//                                  lk::Address target_address(address_at_base58);
//                                  subscribe_account_info(_web_socket_client, target_address);
//                              }
//                              catch (const base::Error& e) {
//                                  out << "can't execute account_info";
//                                  LOG_ERROR << "can't execute account_info:" << e.what();
//                              }
//                          },
//                          "subscribe on updates account info by specific address",
//                          { "address at base58" }));

//     _connected_mode_commands.push_back(
//       _root_menu->Insert("unsubscribe_account_info",
//                          [this](std::ostream& out, std::string address_at_base58) {
//                              try {
//                                  lk::Address target_address(address_at_base58);
//                                  unsubscribe_account_info(_web_socket_client, target_address);
//                              }
//                              catch (const base::Error& e) {
//                                  out << "can't execute account_info";
//                                  LOG_ERROR << "can't execute account_info:" << e.what();
//                              }
//                          },
//                          "unsubscribe from account info updates by specific address",
//                          { "address at base58" }));

//     _connected_mode_commands.push_back(_root_menu->Insert("find_transaction",
//                                                           [this](std::ostream& out, std::string tx_hash_at_hex) {
//                                                               try {
//                                                                   auto hash =
//                                                                   base::Sha256::fromHex(tx_hash_at_hex);
//                                                                   call_find_transaction(_web_socket_client,
//                                                                   hash);
//                                                               }
//                                                               catch (const base::Error& e) {
//                                                                   out << "can't execute find_transaction";
//                                                                   LOG_ERROR << "can't execute find_transaction:"
//                                                                             << e.what();
//                                                               }
//                                                           },
//                                                           "get transaction by specific hash",
//                                                           { "transaction hash at hex" }));

//     _connected_mode_commands.push_back(
//       _root_menu->Insert("find_transaction_status",
//                          [this](std::ostream& out, std::string tx_hash_at_hex) {
//                              try {
//                                  auto hash = base::Sha256::fromHex(tx_hash_at_hex);
//                                  call_find_transaction_status(_web_socket_client, hash);
//                              }
//                              catch (const base::Error& e) {
//                                  out << "can't execute find_transaction_status";
//                                  LOG_ERROR << "can't execute find_transaction_status:" << e.what();
//                              }
//                          },
//                          "get transaction status by specific hash",
//                          { "transaction hash at hex" }));

//     _connected_mode_commands.push_back(_root_menu->Insert("find_block",
//                                                           [this](std::ostream& out, std::string
//                                                           block_hash_at_hex) {
//                                                               try {
//                                                                   auto hash =
//                                                                   base::Sha256::fromHex(block_hash_at_hex);
//                                                                   call_find_block(_web_socket_client, hash);
//                                                               }
//                                                               catch (const base::Error& e) {
//                                                                   out << "can't execute find_block";
//                                                                   LOG_ERROR << "can't execute find_block:" <<
//                                                                   e.what();
//                                                               }
//                                                           },
//                                                           "get block by specific hash",
//                                                           { "block hash at hex" }));

//     _connected_mode_commands.push_back(
//       _root_menu->Insert("transfer",
//                          [this](std::ostream& out,
//                                 std::string key_path,
//                                 std::string recipient_address_at_base58_str,
//                                 std::string fee_str,
//                                 std::string amount_str) {
//                              try {
//                                  lk::Address to_address{ recipient_address_at_base58_str };
//                                  auto amount = websocket::deserializeBalance(amount_str);
//                                  auto fee = websocket::deserializeFee(fee_str);
//                                  std::filesystem::path keys_dir{ key_path };
//                                  if (std::filesystem::exists(keys_dir)) {
//                                      transfer(out, _web_socket_client, to_address, amount, fee, keys_dir);
//                                  }
//                                  else {
//                                      out << "can't parse input data for transfer";
//                                      LOG_ERROR << "can't parse input data for transfer";
//                                  }
//                              }
//                              catch (const base::Error& e) {
//                                  out << "can't execute transfer";
//                                  LOG_ERROR << "can't execute transfer:" << e.what();
//                              }
//                          },
//                          "transfer coins to specific address",
//                          { "path to folder with key",
//                            "address of recipient at base58",
//                            "fee for transaction",
//                            "amount of coins for transfer" }));

//     _connected_mode_commands.push_back(
//       _root_menu->Insert("contract_call",
//                          [this](std::ostream& out,
//                                 std::string key_path,
//                                 std::string contract_address_at_base58,
//                                 std::string fee_str,
//                                 std::string amount_str,
//                                 std::string message) {
//                              try {
//                                  lk::Address to_address{ contract_address_at_base58 };
//                                  auto amount = websocket::deserializeBalance(amount_str);
//                                  auto fee = websocket::deserializeFee(fee_str);
//                                  std::filesystem::path keys_dir{ key_path };
//                                  if (std::filesystem::exists(keys_dir) && !message.empty()) {
//                                      contract_call(out, _web_socket_client, to_address, amount, fee, keys_dir,
//                                      message);
//                                  }
//                                  else {
//                                      out << "can't parse input data for contract_call";
//                                      LOG_ERROR << "can't parse input data for contract_call";
//                                  }
//                              }
//                              catch (const base::Error& e) {
//                                  out << "can't execute transfer";
//                                  LOG_ERROR << "can't execute transfer:" << e.what();
//                              }
//                          },
//                          "call deployed contract",
//                          { "path to folder with key",
//                            "address of contract at base58",
//                            "fee for contract call",
//                            "amount of coins for call",
//                            "message for call at hex" }));

//     _connected_mode_commands.push_back(_root_menu->Insert(
//       "push_contract",
//       [this](std::ostream& out,
//              std::string key_path,
//              std::string fee_str,
//              std::string amount_str,
//              std::string path_to_compiled_folder,
//              std::string init_message) {
//           try {
//               auto amount = websocket::deserializeBalance(amount_str);
//               auto fee = websocket::deserializeFee(fee_str);
//               std::filesystem::path keys_dir{ key_path };
//               if (std::filesystem::exists(keys_dir)) {
//                   push_contract(out, _web_socket_client, amount, fee, keys_dir, path_to_compiled_folder,
//                   init_message);
//               }
//               else {
//                   out << "can't parse input data for push_contract";
//                   LOG_ERROR << "can't parse input data for push_contract";
//               }
//           }
//           catch (const base::Error& e) {
//               out << "can't execute transfer";
//               LOG_ERROR << "can't execute transfer:" << e.what();
//           }
//       },
//       "deploy compiled contract",
//       { "path to folder with key",
//         "fee for contract call",
//         "amount of coins for call",
//         "path to compiled contract data files",
//         "message for initializing contract at hex" }));

//     _connected_mode_commands.push_back(_root_menu->Insert(
//       "disconnect",
//       [this]([[maybe_unused]] std::ostream& out) { _web_socket_client.disconnect(); },
//       "disconnect client from likelib node"));

//     disableCommands(_connected_mode_commands);
//     enableCommands(_disconnected_mode_commands);
// }
