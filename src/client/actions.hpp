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

void call_last_block_info(websocket::WebSocketClient& client);

void call_account_info(websocket::WebSocketClient& client, const lk::Address& address);

void subscribe_account_info(websocket::WebSocketClient& client, const lk::Address& address);

void unsubscribe_account_info(websocket::WebSocketClient& client, const lk::Address& address);

void call_find_transaction(websocket::WebSocketClient& client, const base::Sha256& hash);

void call_find_transaction_status(websocket::WebSocketClient& client, const base::Sha256& hash);

void call_find_block(websocket::WebSocketClient& client, const base::Sha256& hash);

void transfer(Client& client,
              websocket::WebSocketClient& web_socket,
              const lk::Address& to_address,
              const lk::Balance& amount,
              const lk::Fee& fee,
              const std::filesystem::path& keys_dir);

void contract_call(Client& client,
                   websocket::WebSocketClient& web_socket,
                   const lk::Address& to_address,
                   const lk::Balance& amount,
                   const lk::Fee& fee,
                   const std::filesystem::path& keys_dir,
                   const std::string& message);

void push_contract(Client& client,
                   websocket::WebSocketClient& web_socket,
                   const lk::Balance& amount,
                   const lk::Fee& fee,
                   const std::filesystem::path& keys_dir,
                   const std::filesystem::path& path_to_compiled_folder,
                   const std::string& message);

void subscribe_last_block_info(websocket::WebSocketClient& client);

void unsubscribe_last_block_info(websocket::WebSocketClient& client);
