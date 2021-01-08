#pragma once

#include "websocket/client.hpp"
#include "websocket/tools.hpp"

#include "core/address.hpp"
#include "core/types.hpp"

#include "client.hpp"

#include <string_view>

void help(Client& client);

void compile_solidity_code(Client& client, const std::string& code_file_path);

void encode_message(Client& client, const std::string& compiled_contract_folder_path, const std::string& message);

void decode_message(Client& client, const std::string& compiled_contract_folder_path, const std::string& message);

void generate_keys(Client& client, const std::string& path);

void keys_info(Client& client, const std::string& path);

void call_last_block_info(websocket::WebSocketClient& web_socket);

void call_account_info(Client& client, websocket::WebSocketClient& web_socket, const std::string& address);

void subscribe_account_info(Client& client, websocket::WebSocketClient& web_socket, const std::string& address);

void unsubscribe_account_info(Client& client, websocket::WebSocketClient& web_socket, const std::string& address);

void call_find_transaction(Client& client, websocket::WebSocketClient& web_socket, const std::string& hash);

void call_find_transaction_status(Client& client, websocket::WebSocketClient& web_socket, const std::string& hash);

void call_find_block(Client& client, websocket::WebSocketClient& web_socket, const std::string& hash);

void transfer(Client& client,
              websocket::WebSocketClient& web_socket,
              const std::string& to_address,
              const std::string& amount,
              const std::string& fee,
              const std::string& keys_dir);

void contract_call(Client& client,
                   websocket::WebSocketClient& web_socket,
                   const std::string& to_address,
                   const std::string& amount,
                   const std::string& fee,
                   const std::string& keys_dir,
                   const std::string& message);

void push_contract(Client& client,
                   websocket::WebSocketClient& web_socket,
                   const std::string& amount,
                   const std::string& fee,
                   const std::string& keys_dir,
                   const std::string& path_to_compiled_folder,
                   const std::string& message);

void subscribe_last_block_info(websocket::WebSocketClient& web_socket);

void unsubscribe_last_block_info(websocket::WebSocketClient& web_socket);
