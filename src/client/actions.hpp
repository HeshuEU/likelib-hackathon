#pragma once

#include "websocket/client.hpp"
#include "websocket/tools.hpp"

#include "core/address.hpp"
#include "core/types.hpp"

#include "client.hpp"

#include <string_view>

void help(Client& client);

void compile_solidity_code(Client& client, const std::vector<std::string>& arguments);

void encode_message(Client& client, const std::vector<std::string>& arguments);

void decode_message(Client& client, const std::vector<std::string>& arguments);

void generate_keys(Client& client, const std::vector<std::string>& arguments);

void keys_info(Client& client, const std::vector<std::string>& arguments);

void add_wallet(Client& client, const std::vector<std::string>& arguments);

void delete_wallet(Client& client, const std::vector<std::string>& arguments);

void show_wallets(Client& client, const std::vector<std::string>& arguments);

void call_last_block_info(Client& client);

void call_account_info(Client& client, const std::vector<std::string>& arguments);

void subscribe_account_info(Client& client, const std::vector<std::string>& arguments);

void unsubscribe_account_info(Client& client, const std::vector<std::string>& arguments);

void call_find_transaction(Client& client, const std::vector<std::string>& arguments);

void call_find_transaction_status(Client& client, const std::vector<std::string>& arguments);

void call_find_block(Client& client, const std::vector<std::string>& arguments);

void transfer(Client& client, const std::vector<std::string>& arguments);

void contract_call(Client& client, const std::vector<std::string>& arguments);

void push_contract(Client& client, const std::vector<std::string>& arguments);

void subscribe_last_block_info(Client& client);

void unsubscribe_last_block_info(Client& client);
