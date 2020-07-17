#include "client.hpp"
#include "actions.hpp"

#include <cli/clilocalsession.h>
#include <cli/remotecli.h>

Client::Client()
  : _root_menu{ std::make_unique<cli::Menu>("empty") }
  , _io_context{}
  , _web_socket_client{ _io_context,
                        std::bind(&Client::printReceivedData, this, std::placeholders::_1, std::placeholders::_2),
                        [this]() {
                            cli::Cli::cout() << "Disconnected from likelib node\n";
                            disableCommands(_connected_mode_commands);
                            enableCommands(_disconnected_mode_commands);
                        } }
{}


void Client::run()
{
    setupCli();
    cli::Cli cli(std::move(_root_menu));
    cli::SetColor();

    cli::CliLocalSession session(cli, _io_context, std::cout, 200);
    session.ExitAction([this](auto& out) {
        out << "Closing App...\n";
        _io_context.stop();
    });
    cli::CliTelnetServer server(_io_context, 5000, cli);
    // exit action for all the connections
    server.ExitAction([](auto& out) { out << "Terminating this session...\n"; });

    _io_context.run();
}


void Client::printReceivedData(websocket::Command::Id command_id, base::PropertyTree received_message)
{
    cli::Cli::cout() << "Received data: " << received_message.toString();
    // TODO
}


void Client::disableCommands(std::vector<cli::CmdHandler>& commands)
{
    for (cli::CmdHandler& command : commands) {
        command.Disable();
    }
}


void Client::enableCommands(std::vector<cli::CmdHandler>& commands)
{
    for (cli::CmdHandler& command : commands) {
        command.Enable();
    }
}


void Client::setupCli()
{
    _disconnected_mode_commands.clear();
    _always_mode_commands.clear();
    _connected_mode_commands.clear();

    _root_menu = std::make_unique<cli::Menu>("likelib");

    _disconnected_mode_commands.push_back(_root_menu->Insert("connect",
                                                             [this](std::ostream& out, std::string host) {
                                                                 if (_web_socket_client.connect(host)) {
                                                                     out << "Connected to node: " << host << "\n";
                                                                     disableCommands(_disconnected_mode_commands);
                                                                     enableCommands(_connected_mode_commands);
                                                                 }
                                                                 else {
                                                                     out << "can't connect to node: " << host << "\n";
                                                                 }
                                                             },
                                                             "Connect client to specific likelib node",
                                                             { "ip and port of likelib node" }));

    _always_mode_commands.push_back(_root_menu->Insert(
      "compile",
      [](std::ostream& out, std::string code_file_path) { compile_solidity_code(out, code_file_path); },
      "compile solidity code to binary format",
      { "path to solidity file for compilation" }));

    _always_mode_commands.push_back(
      _root_menu->Insert("encode",
                         [](std::ostream& out, std::string compiled_contract_folder_path, std::string message) {
                             encode_message(out, compiled_contract_folder_path, message);
                         },
                         "encode message for contract call",
                         { "path to compiled contract data files", "message for encode" }));

    _always_mode_commands.push_back(
      _root_menu->Insert("decode",
                         [](std::ostream& out, std::string compiled_contract_folder_path, std::string message) {
                             decode_message(out, compiled_contract_folder_path, message);
                         },
                         "decode message which was returned by contract call",
                         { "path to compiled contract data files", "message for decode" }));

    _always_mode_commands.push_back(
      _root_menu->Insert("keys_generate",
                         [](std::ostream& out, std::string path) { generate_keys(out, path); },
                         "generate new key and store to specific folder",
                         { "path to folder to save key" }));

    _always_mode_commands.push_back(
      _root_menu->Insert("keys_info",
                         [](std::ostream& out, std::string path) { keys_info(out, path); },
                         "print info about specified key",
                         { "path to folder with key" }));

    _connected_mode_commands.push_back(_root_menu->Insert(
      "last_block_info",
      [this]([[maybe_unused]] std::ostream& out) { call_last_block_info(_web_socket_client); },
      "get last block info"));

    _connected_mode_commands.push_back(_root_menu->Insert(
      "subscribe_last_block_info",
      [this]([[maybe_unused]] std::ostream& out) { subscribe_last_block_info(_web_socket_client); },
      "get last block info when apeared new block"));

    _connected_mode_commands.push_back(_root_menu->Insert(
      "unsubscribe_last_block_info",
      [this]([[maybe_unused]] std::ostream& out) { unsubscribe_last_block_info(_web_socket_client); },
      "get last block info when apeared new block"));

    _connected_mode_commands.push_back(_root_menu->Insert("account_info",
                                                          [this](std::ostream& out, std::string address_at_base58) {
                                                              try {
                                                                  lk::Address target_address(address_at_base58);
                                                                  call_account_info(_web_socket_client, target_address);
                                                              }
                                                              catch (const base::Error& e) {
                                                                  out << "can't execute account_info";
                                                                  LOG_ERROR << "can't execute account_info:"
                                                                            << e.what();
                                                              }
                                                          },
                                                          "get account info by specific address",
                                                          { "address at base58" }));

    _connected_mode_commands.push_back(
      _root_menu->Insert("subscribe_account_info",
                         [this](std::ostream& out, std::string address_at_base58) {
                             try {
                                 lk::Address target_address(address_at_base58);
                                 subscribe_account_info(_web_socket_client, target_address);
                             }
                             catch (const base::Error& e) {
                                 out << "can't execute account_info";
                                 LOG_ERROR << "can't execute account_info:" << e.what();
                             }
                         },
                         "subscribe on updates account info by specific address",
                         { "address at base58" }));

    _connected_mode_commands.push_back(
      _root_menu->Insert("unsubscribe_account_info",
                         [this](std::ostream& out, std::string address_at_base58) {
                             try {
                                 lk::Address target_address(address_at_base58);
                                 unsubscribe_account_info(_web_socket_client, target_address);
                             }
                             catch (const base::Error& e) {
                                 out << "can't execute account_info";
                                 LOG_ERROR << "can't execute account_info:" << e.what();
                             }
                         },
                         "unsubscribe from account info updates by specific address",
                         { "address at base58" }));

    _connected_mode_commands.push_back(_root_menu->Insert("find_transaction",
                                                          [this](std::ostream& out, std::string tx_hash_at_hex) {
                                                              try {
                                                                  auto hash = base::Sha256::fromHex(tx_hash_at_hex);
                                                                  call_find_transaction(_web_socket_client, hash);
                                                              }
                                                              catch (const base::Error& e) {
                                                                  out << "can't execute find_transaction";
                                                                  LOG_ERROR << "can't execute find_transaction:"
                                                                            << e.what();
                                                              }
                                                          },
                                                          "get transaction by specific hash",
                                                          { "transaction hash at hex" }));

    _connected_mode_commands.push_back(
      _root_menu->Insert("find_transaction_status",
                         [this](std::ostream& out, std::string tx_hash_at_hex) {
                             try {
                                 auto hash = base::Sha256::fromHex(tx_hash_at_hex);
                                 call_find_transaction_status(_web_socket_client, hash);
                             }
                             catch (const base::Error& e) {
                                 out << "can't execute find_transaction_status";
                                 LOG_ERROR << "can't execute find_transaction_status:" << e.what();
                             }
                         },
                         "get transaction status by specific hash",
                         { "transaction hash at hex" }));

    _connected_mode_commands.push_back(_root_menu->Insert("find_block",
                                                          [this](std::ostream& out, std::string block_hash_at_hex) {
                                                              try {
                                                                  auto hash = base::Sha256::fromHex(block_hash_at_hex);
                                                                  call_find_block(_web_socket_client, hash);
                                                              }
                                                              catch (const base::Error& e) {
                                                                  out << "can't execute find_block";
                                                                  LOG_ERROR << "can't execute find_block:" << e.what();
                                                              }
                                                          },
                                                          "get block by specific hash",
                                                          { "block hash at hex" }));

    _connected_mode_commands.push_back(
      _root_menu->Insert("transfer",
                         [this](std::ostream& out,
                                std::string key_path,
                                std::string recipient_address_at_base58_str,
                                std::string fee_str,
                                std::string amount_str) {
                             try {
                                 lk::Address to_address{ recipient_address_at_base58_str };
                                 auto amount = websocket::deserializeBalance(amount_str);
                                 auto fee = websocket::deserializeFee(fee_str);
                                 std::filesystem::path keys_dir{ key_path };
                                 if (amount && fee && std::filesystem::exists(keys_dir)) {
                                     transfer(out, _web_socket_client, to_address, *amount, *fee, keys_dir);
                                 }
                                 else {
                                     out << "can't parse input data for transfer";
                                     LOG_ERROR << "can't parse input data for transfer";
                                 }
                             }
                             catch (const base::Error& e) {
                                 out << "can't execute transfer";
                                 LOG_ERROR << "can't execute transfer:" << e.what();
                             }
                         },
                         "transfer coins to specific address",
                         { "path to folder with key",
                           "address of recipient at base58",
                           "fee for transaction",
                           "amount of coins for transfer" }));

    _connected_mode_commands.push_back(_root_menu->Insert(
      "contract_call",
      [this](std::ostream& out,
             std::string key_path,
             std::string contract_address_at_base58,
             std::string fee_str,
             std::string amount_str,
             std::string message) {
          try {
              lk::Address to_address{ contract_address_at_base58 };
              auto amount = websocket::deserializeBalance(amount_str);
              auto fee = websocket::deserializeFee(fee_str);
              std::filesystem::path keys_dir{ key_path };
              if (amount && fee && std::filesystem::exists(keys_dir) && !message.empty()) {
                  contract_call(out, _web_socket_client, to_address, *amount, *fee, keys_dir, message);
              }
              else {
                  out << "can't parse input data for contract_call";
                  LOG_ERROR << "can't parse input data for contract_call";
              }
          }
          catch (const base::Error& e) {
              out << "can't execute transfer";
              LOG_ERROR << "can't execute transfer:" << e.what();
          }
      },
      "call deployed contract",
      { "path to folder with key",
        "address of contract at base58",
        "fee for contract call",
        "amount of coins for call",
        "message for call at hex" }));

    _connected_mode_commands.push_back(_root_menu->Insert(
      "push_contract",
      [this](std::ostream& out,
             std::string key_path,
             std::string fee_str,
             std::string amount_str,
             std::string path_to_compiled_folder,
             std::string init_message) {
          try {
              auto amount = websocket::deserializeBalance(amount_str);
              auto fee = websocket::deserializeFee(fee_str);
              std::filesystem::path keys_dir{ key_path };
              if (amount && fee && std::filesystem::exists(keys_dir)) {
                  push_contract(
                    out, _web_socket_client, *amount, *fee, keys_dir, path_to_compiled_folder, init_message);
              }
              else {
                  out << "can't parse input data for push_contract";
                  LOG_ERROR << "can't parse input data for push_contract";
              }
          }
          catch (const base::Error& e) {
              out << "can't execute transfer";
              LOG_ERROR << "can't execute transfer:" << e.what();
          }
      },
      "deploy compiled contract",
      { "path to folder with key",
        "fee for contract call",
        "amount of coins for call",
        "path to compiled contract data files",
        "message for initializing contract at hex" }));

    _connected_mode_commands.push_back(_root_menu->Insert(
      "disconnect",
      [this]([[maybe_unused]] std::ostream& out) { _web_socket_client.disconnect(); },
      "disconnect client from likelib node"));

    disableCommands(_connected_mode_commands);
    enableCommands(_disconnected_mode_commands);
}
