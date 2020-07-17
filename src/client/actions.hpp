#pragma once

#include "websocket/client.hpp"
#include "websocket/tools.hpp"

#include "core/address.hpp"
#include "core/types.hpp"

#include <string_view>

void compile_solidity_code(std::ostream& output, const std::string& code_file_path);

void encode_message(std::ostream& output, const std::string& compiled_contract_folder_path, const std::string& message);

void decode_message(std::ostream& output, const std::string& compiled_contract_folder_path, const std::string& message);

void generate_keys(std::ostream& output, const std::string& path);

void keys_info(std::ostream& output, const std::string& path);

void call_last_block_info(websocket::WebSocketClient& client);

void call_account_info(websocket::WebSocketClient& client, const lk::Address& address);

void subscribe_account_info(websocket::WebSocketClient& client, const lk::Address& address);

void unsubscribe_account_info(websocket::WebSocketClient& client, const lk::Address& address);

void call_find_transaction(websocket::WebSocketClient& client, const base::Sha256& hash);

void call_find_transaction_status(websocket::WebSocketClient& client, const base::Sha256& hash);

void call_find_block(websocket::WebSocketClient& client, const base::Sha256& hash);

void transfer(std::ostream& output,
              websocket::WebSocketClient& client,
              const lk::Address& to_address,
              const lk::Balance& amount,
              const lk::Fee& fee,
              const std::filesystem::path& keys_dir);

void contract_call(std::ostream& output,
                   websocket::WebSocketClient& client,
                   const lk::Address& to_address,
                   const lk::Balance& amount,
                   const lk::Fee& fee,
                   const std::filesystem::path& keys_dir,
                   const std::string& message);

void push_contract(std::ostream& output,
                   websocket::WebSocketClient& client,
                   const lk::Balance& amount,
                   const lk::Fee& fee,
                   const std::filesystem::path& keys_dir,
                   const std::filesystem::path& path_to_compiled_folder,
                   const std::string& message);

void subscribe_last_block_info(websocket::WebSocketClient& client);

void unsubscribe_last_block_info(websocket::WebSocketClient& client);
