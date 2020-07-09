#include "actions.hpp"
#include "config.hpp"
#include "utility.hpp"

#include "core/types.hpp"

#include "websocket/client.hpp"
#include "websocket/tools.hpp"

#include "base/error.hpp"
#include "base/program_options.hpp"

#include <cli/cli.h>
#include <cli/cliasyncsession.h>

#include <vector>


void print_received_data(websocket::Command::Id command_id, base::PropertyTree received_message)
{
    cli::Cli::cout() << "Received data: " << received_message.toString();
    // TODO
}


void disable(std::vector<cli::CmdHandler>& commands)
{
    for (cli::CmdHandler& command : commands) {
        command.Disable();
    }
}


void enable(std::vector<cli::CmdHandler>& commands)
{
    for (cli::CmdHandler& command : commands) {
        command.Enable();
    }
}

void client_process()
{
    std::vector<cli::CmdHandler> disconnected_mode_commands{};
    std::vector<cli::CmdHandler> always_mode_commands{};
    std::vector<cli::CmdHandler> connected_mode_commands{};

    boost::asio::io_context ios;
    websocket::WebSocketClient web_socket_client(
      ios, &print_received_data, [&disconnected_mode_commands, &connected_mode_commands]() {
          cli::Cli::cout() << "Disconnected from likelib node\n";
          disable(connected_mode_commands);
          enable(disconnected_mode_commands);
      });

    auto root_menu = std::make_unique<cli::Menu>("likelib");

    disconnected_mode_commands.push_back(root_menu->Insert(
      "connect",
      [&web_socket_client, &disconnected_mode_commands, &connected_mode_commands](std::ostream& out, std::string host) {
          if (web_socket_client.connect(host)) {
              out << "Connected to node: " << host << "\n";
              disable(disconnected_mode_commands);
              enable(connected_mode_commands);
          }
          else {
              out << "can't connect to node: " << host << "\n";
          }
      },
      "Connect client to specific likelib node",
      { "ip and port of likelib node" }));

    always_mode_commands.push_back(root_menu->Insert(
      "compile",
      [](std::ostream& out, std::string code_file_path) { compile_solidity_code(out, code_file_path); },
      "compile solidity code to binary format",
      { "path to solidity file for compilation" }));

    always_mode_commands.push_back(
      root_menu->Insert("encode",
                        [](std::ostream& out, std::string compiled_contract_folder_path, std::string message) {
                            encode_message(out, compiled_contract_folder_path, message);
                        },
                        "encode message for contract call",
                        { "path to compiled contract data files", "message for encode" }));

    always_mode_commands.push_back(
      root_menu->Insert("decode",
                        [](std::ostream& out, std::string compiled_contract_folder_path, std::string message) {
                            decode_message(out, compiled_contract_folder_path, message);
                        },
                        "decode message which was returned by contract call",
                        { "path to compiled contract data files", "message for decode" }));

    always_mode_commands.push_back(
      root_menu->Insert("keys_generate",
                        [](std::ostream& out, std::string path) { generate_keys(out, path); },
                        "generate new key and store to specific folder",
                        { "path to folder to save key" }));

    always_mode_commands.push_back(root_menu->Insert("keys_info",
                                                     [](std::ostream& out, std::string path) { keys_info(out, path); },
                                                     "print info about specified key",
                                                     { "path to folder with key" }));

    connected_mode_commands.push_back(root_menu->Insert(
      "last_block_info",
      [&web_socket_client]([[maybe_unused]] std::ostream& out) { call_last_block_info(web_socket_client); },
      "get last block info"));

    connected_mode_commands.push_back(root_menu->Insert(
      "subscribe_last_block_info",
      [&web_socket_client]([[maybe_unused]] std::ostream& out) { subscribe_last_block_info(web_socket_client); },
      "get last block info when apeared new block"));

    connected_mode_commands.push_back(
      root_menu->Insert("account_info",
                        [&web_socket_client](std::ostream& out, std::string address_at_base58) {
                            try {
                                lk::Address target_address(address_at_base58);
                                call_account_info(web_socket_client, target_address);
                            }
                            catch (const base::Error& e) {
                                out << "can't execute account_info";
                                LOG_ERROR << "can't execute account_info:" << e.what();
                            }
                        },
                        "get account info by specific address",
                        { "address at base58" }));

    connected_mode_commands.push_back(
      root_menu->Insert("find_transaction",
                        [&web_socket_client](std::ostream& out, std::string tx_hash_at_hex) {
                            try {
                                auto hash = base::Sha256::fromHex(tx_hash_at_hex);
                                call_find_transaction(web_socket_client, hash);
                            }
                            catch (const base::Error& e) {
                                out << "can't execute find_transaction";
                                LOG_ERROR << "can't execute find_transaction:" << e.what();
                            }
                        },
                        "get transaction by specific hash",
                        { "transaction hash at hex" }));

    connected_mode_commands.push_back(
      root_menu->Insert("find_transaction_status",
                        [&web_socket_client](std::ostream& out, std::string tx_hash_at_hex) {
                            try {
                                auto hash = base::Sha256::fromHex(tx_hash_at_hex);
                                call_find_transaction_status(web_socket_client, hash);
                            }
                            catch (const base::Error& e) {
                                out << "can't execute find_transaction_status";
                                LOG_ERROR << "can't execute find_transaction_status:" << e.what();
                            }
                        },
                        "get transaction status by specific hash",
                        { "transaction hash at hex" }));

    connected_mode_commands.push_back(
      root_menu->Insert("find_block",
                        [&web_socket_client](std::ostream& out, std::string block_hash_at_hex) {
                            try {
                                auto hash = base::Sha256::fromHex(block_hash_at_hex);
                                call_find_block(web_socket_client, hash);
                            }
                            catch (const base::Error& e) {
                                out << "can't execute find_block";
                                LOG_ERROR << "can't execute find_block:" << e.what();
                            }
                        },
                        "get block by specific hash",
                        { "block hash at hex" }));

    connected_mode_commands.push_back(
      root_menu->Insert("transfer",
                        [&web_socket_client](std::ostream& out,
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
                                    transfer(out, web_socket_client, to_address, *amount, *fee, keys_dir);
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

    connected_mode_commands.push_back(
      root_menu->Insert("contract_call",
                        [&web_socket_client](std::ostream& out,
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
                                    contract_call(out, web_socket_client, to_address, *amount, *fee, keys_dir, message);
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

    connected_mode_commands.push_back(root_menu->Insert(
      "push_contract",
      [&web_socket_client](std::ostream& out,
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
                  push_contract(out, web_socket_client, *amount, *fee, keys_dir, path_to_compiled_folder, init_message);
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

    connected_mode_commands.push_back(root_menu->Insert(
      "disconnect",
      [&web_socket_client]([[maybe_unused]] std::ostream& out) { web_socket_client.disconnect(); },
      "disconnect client from likelib node"));

    cli::Cli cli(std::move(root_menu));
    cli::SetColor();

    cli::CliAsyncSession session(ios, cli);
    session.ExitAction([&ios](auto& out) // session exit action
                       {
                           out << "Closing App...\n";
                           ios.stop();
                       });

    disable(connected_mode_commands);
    enable(disconnected_mode_commands);

    ios.run();
}


int main(int argc, const char** argv)
{
    base::initLog(base::Sink::FILE);
    base::ProgramOptionsParser options_parser;
    options_parser.addFlag("version,v", "Print version of program");
    options_parser.process(argc, argv);

    if (options_parser.hasOption("help")) {
        std::cout << options_parser.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if (options_parser.hasOption("version")) {
        std::cout << "Likelib client v" << config::CLIENT_VERSION << std::endl;
        return base::config::EXIT_OK;
    }

    client_process();

    return base::config::EXIT_OK;
}