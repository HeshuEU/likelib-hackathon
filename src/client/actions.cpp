#include "actions.hpp"
#include "config.hpp"

#include "core/transaction.hpp"

#include "websocket/error.hpp"
#include "websocket/tools.hpp"

#include "vm/tools.hpp"
#include "vm/vm.hpp"

#include "base/config.hpp"
#include "base/directory.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"
#include "base/time.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

std::optional<base::Bytes> takeMessage(Client& client, const std::string message)
{
    base::Bytes message_bytes;
    try {
        message_bytes = base::fromHex<base::Bytes>(message);
    }
    catch (const base::InvalidArgument& er) {
        client.output("Wrong message was entered");
        return {};
    }
    return message_bytes;
}


std::optional<lk::Address> takeAddress(Client& client, const std::string address)
{
    base::FixedBytes<lk::Address::LENGTH_IN_BYTES> address_bytes;
    try {
        address_bytes = base::FixedBytes<lk::Address::LENGTH_IN_BYTES>(base::base58Decode(address));
    }
    catch (const base::InvalidArgument& er) {
        client.output("Wrong address was entered");
        return {};
    }
    return lk::Address{ address_bytes };
}


std::optional<lk::Fee> takeFee(Client& client, const std::string fee)
{
    for (auto c : fee) {
        if (!std::isdigit(c)) {
            client.output("Wrong fee was entered");
            return {};
        }
    }
    return lk::Fee{ std::stoull(fee) };
}


std::optional<lk::Balance> takeAmount(Client& client, const std::string amount_str)
{
    lk::Balance amount;
    try {
        amount = lk::Balance{ amount_str };
    }
    catch (const boost::wrapexcept<std::runtime_error>& er) {
        client.output("Wrong amount was entered");
        return {};
    }
    return amount;
}


std::optional<base::Sha256> takeHash(Client& client, const std::string hash)
{
    base::FixedBytes<base::Sha256::LENGTH> hash_bytes;
    try {
        hash_bytes = base::FixedBytes<base::Sha256::LENGTH>(base::fromHex<base::Bytes>(hash));
    }
    catch (const base::InvalidArgument& er) {
        client.output("Wrong hash was entered");
        return {};
    }
    return base::Sha256{ hash_bytes };
}

void compile_solidity_code(Client& client, const std::vector<std::string>& arguments)
{
    std::optional<vm::Contracts> contracts;
    std::string code_file_path{ arguments[0] };
    try {
        contracts = vm::compile(code_file_path);
    }
    catch (const base::ParsingError& er) {
        LOG_ERROR << er.what();
        return;
    }
    catch (const base::SystemCallFailed& er) {
        LOG_ERROR << er.what();
        return;
    }

    if (!contracts) {
        LOG_ERROR << "Compilation error\n";
        return;
    }

    std::stringstream output;
    output << "Compiled contracts:\n";
    for (const auto& contract : contracts.value()) {
        output << "\t" << contract.name << "\n";
        try {
            std::filesystem::path current_folder{ contract.name };
            base::createIfNotExists(current_folder);

            {
                std::ofstream file;
                file.open(current_folder / std::filesystem::path{ config::CONTRACT_BINARY_FILE });
                file << base::toHex(contract.code);
            }

            {
                std::ofstream file;
                file.open(current_folder / std::filesystem::path{ config::METADATA_JSON_FILE });
                contract.metadata.serialize(file);
            }
        }
        catch (const base::Error& er) {
            client.output(output.str());
            LOG_ERROR << er.what();
            return;
        }
        catch (...) {
            client.output(output.str());
            LOG_ERROR << "unexpected error at saving contract:" << contract.name;
            return;
        }
        client.output(output.str());
    }
}


void encode_message(Client& client, const std::vector<std::string>& arguments)
{
    std::string compiled_contract_folder_path{ arguments[0] };
    std::string message{ arguments[1] };
    try {
        auto output_message = vm::encodeCall(compiled_contract_folder_path, message);
        if (output_message) {
            client.output(base::toHex(output_message.value()));
        }
        else {
            client.output("encoding failed.\n");
            return;
        }
    }
    catch (const base::ParsingError& er) {
        client.output(er.what());
        return;
    }
    catch (const base::SystemCallFailed& er) {
        client.output(er.what());
        return;
    }
}


void decode_message(Client& client, const std::vector<std::string>& arguments)
{
    std::string compiled_contract_folder_path{ arguments[0] };
    std::string message{ arguments[1] };
    try {
        auto output_message = vm::decodeOutput(compiled_contract_folder_path, message);
        if (output_message) {
            client.output(output_message.value());
        }
        else {
            client.output("decoding failed.\n");
            return;
        }
    }
    catch (const base::ParsingError& er) {
        client.output(er.what());
        return;
    }
    catch (const base::SystemCallFailed& er) {
        client.output(er.what());
        return;
    }
}


void generate_keys(Client& client, const std::vector<std::string>& arguments)
{
    std::string path{ arguments[0] };
    const auto& priv = base::Secp256PrivateKey();

    auto private_path = base::config::makePrivateKeyPath(path);
    std::stringstream output;
    if (std::filesystem::exists(private_path)) {
        output << "Error: " << private_path << " already exists.\n";
        client.output(output.str());
        LOG_ERROR << private_path << " file already exists";
        return;
    }

    priv.save(private_path);

    output << "Generated key at " << path << std::endl;
    output << "Address: " << lk::Address(priv.toPublicKey()) << std::endl;
    output << "Hash of public key: " << base::Sha256::compute(priv.toPublicKey().toBytes()) << std::endl;
    output << "Hash of private key: " << base::Sha256::compute(priv.getBytes().toBytes()) << std::endl;
    client.output(output.str());
    LOG_INFO << "Generated key at " << path;

    priv.save(private_path);
}


void keys_info(Client& client, const std::vector<std::string>& arguments)
{
    std::string path{ arguments[0] };
    auto private_path = base::config::makePrivateKeyPath(path);
    std::stringstream output;
    if (!std::filesystem::exists(private_path)) {
        output << "Error: " << private_path << " doesn't exist.\n";
        client.output(output.str());
        LOG_ERROR << private_path << " file not exists";
        return;
    }

    auto priv = base::Secp256PrivateKey::load(private_path);

    output << "Address: " << lk::Address(priv.toPublicKey()) << std::endl;
    output << "Hash of public key: " << base::Sha256::compute(priv.toPublicKey()) << std::endl;
    output << "Hash of private key: " << base::Sha256::compute(priv.getBytes()) << std::endl;
    client.output(output.str());
}


void add_wallet(Client& client, const std::vector<std::string>& arguments)
{
    std::string wallet_name{ arguments[0] };
    std::string keys_dir_str{ arguments[1] };
    auto wallets = client.getWallets();
    std::filesystem::path keys_dir{ keys_dir_str };

    auto private_key_path = base::config::makePrivateKeyPath(keys_dir);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    if (wallets.find(wallet_name) == wallets.end()) {
        // wallets[wallet_name] = keys_dir;
        client.addWallet(wallet_name, keys_dir);
        client.output("Added a wallet with the name " + wallet_name);
    }
    else {
        client.output("Wallet with the name " + wallet_name + " already exists");
    }
}


void delete_wallet(Client& client, const std::vector<std::string>& arguments)
{
    auto wallets = client.getWallets();
    std::string wallet_name{ arguments[0] };

    if (wallets.find(wallet_name) == wallets.end()) {
        client.output("Wallet with the name " + wallet_name + " does not exists");
    }
    else {
        client.deleteWallet(wallet_name);
        client.output("Wallet with the name " + wallet_name + " deleted");
    }
}


void show_wallets(Client& client, const std::vector<std::string>& arguments)
{
    auto wallets = client.getWallets();

    if (wallets.empty()) {
        client.output("no wallets saved");
    }
    else {
        for (const auto& wallet : wallets) {
            client.output(wallet.first + " - " + wallet.second);
        }
    }
}


void call_last_block_info(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }
    auto& web_socket{ client.getWebSocket() };

    LOG_INFO << "last_block_info";
    web_socket.send(websocket::Command::CALL_LAST_BLOCK_INFO, base::json::Value::object());
}


void call_account_info(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string address_str{ arguments[0] };
    auto& web_socket{ client.getWebSocket() };

    auto address = takeAddress(client, address_str);
    if (!address) {
        return;
    }

    LOG_INFO << "account_info for address: " << address.value();
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(address.value());
    web_socket.send(websocket::Command::CALL_ACCOUNT_INFO, std::move(request_args));
}


void subscribe_account_info(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string address_str{ arguments[0] };
    auto& web_socket{ client.getWebSocket() };

    auto address = takeAddress(client, address_str);
    if (!address) {
        return;
    }

    LOG_INFO << "account_info for address: " << address.value();
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(address.value());
    web_socket.send(websocket::Command::SUBSCRIBE_ACCOUNT_INFO, std::move(request_args));
}


void unsubscribe_account_info(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string address_str{ arguments[0] };
    auto& web_socket{ client.getWebSocket() };

    auto address = takeAddress(client, address_str);
    if (!address) {
        return;
    }

    LOG_INFO << "account_info for address: " << address.value();
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(address.value());
    web_socket.send(websocket::Command::UNSUBSCRIBE_ACCOUNT_INFO, std::move(request_args));
}


void call_find_transaction(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string hash_str{ arguments[0] };
    auto& web_socket{ client.getWebSocket() };

    auto hash = takeHash(client, hash_str);
    if (!hash) {
        return;
    }

    LOG_INFO << "find_transaction by hash: " << hash.value();
    auto request_args = base::json::Value::object();
    request_args["hash"] = websocket::serializeHash(hash.value());
    web_socket.send(websocket::Command::CALL_FIND_TRANSACTION, std::move(request_args));
}


void call_find_transaction_status(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string hash_str{ arguments[0] };
    auto& web_socket{ client.getWebSocket() };

    auto hash = takeHash(client, hash_str);
    if (!hash) {
        return;
    }

    LOG_INFO << "find_transaction_status by hash: " << hash.value();
    auto request_args = base::json::Value::object();
    request_args["hash"] = websocket::serializeHash(hash.value());
    web_socket.send(websocket::Command::CALL_FIND_TRANSACTION_STATUS, std::move(request_args));
}


void call_find_block(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string hash_str{ arguments[0] };
    auto& web_socket{ client.getWebSocket() };

    auto hash = takeHash(client, hash_str);
    if (!hash) {
        return;
    }

    LOG_INFO << "find_block by hash: " << hash.value();
    auto request_args = base::json::Value::object();
    request_args["hash"] = websocket::serializeHash(hash.value());
    web_socket.send(websocket::Command::CALL_FIND_BLOCK, std::move(request_args));
}


void transfer(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string keys_dir_str{ arguments[0] };
    std::string to_address_str{ arguments[1] };
    std::string fee_str{ arguments[2] };
    std::string amount_str{ arguments[3] };
    auto& web_socket{ client.getWebSocket() };

    auto to_address = takeAddress(client, to_address_str);
    if (!to_address) {
        return;
    }

    auto amount = takeAmount(client, amount_str);
    if (!amount) {
        return;
    }

    auto fee = takeFee(client, fee_str);
    if (!fee) {
        return;
    }

    auto wallets = client.getWallets();

    std::filesystem::path keys_dir = wallets.find(keys_dir_str) != wallets.end() ? wallets[keys_dir_str] : keys_dir_str;

    auto private_key_path = base::config::makePrivateKeyPath(keys_dir);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(to_address.value());
    txb.setAmount(amount.value());
    txb.setTimestamp(base::Time::now());
    txb.setFee(fee.value());
    txb.setData({});
    auto tx = std::move(txb).build();

    tx.sign(private_key);

    auto tx_hash = tx.hashOfTransaction();
    client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Transfer from " << from_address << " to " << to_address.value() << " with amount " << amount.value();

    auto request_args = websocket::serializeTransaction(tx);
    web_socket.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


void contract_call(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string keys_dir_str{ arguments[0] };
    std::string to_address_str{ arguments[1] };
    std::string fee_str{ arguments[2] };
    std::string amount_str{ arguments[3] };
    std::string message{ arguments[4] };
    auto& web_socket{ client.getWebSocket() };

    auto to_address = takeAddress(client, to_address_str);
    if (!to_address) {
        return;
    }

    auto amount = takeAmount(client, amount_str);
    if (!amount) {
        return;
    }

    auto fee = takeFee(client, fee_str);
    if (!fee) {
        return;
    }

    auto wallets = client.getWallets();

    std::filesystem::path keys_dir = wallets.find(keys_dir_str) != wallets.end() ? wallets[keys_dir_str] : keys_dir_str;

    auto data = takeMessage(client, message);
    if (!data) {
        return;
    }

    auto private_key_path = base::config::makePrivateKeyPath(keys_dir);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(to_address.value());
    txb.setAmount(amount.value());
    txb.setTimestamp(base::Time::now());
    txb.setFee(fee.value());
    txb.setData(data.value());
    auto tx = std::move(txb).build();

    tx.sign(private_key);

    auto tx_hash = tx.hashOfTransaction();
    client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Contract_call from " << from_address << ", to " << to_address.value() << ", amount " << amount.value()
             << ",fee " << fee.value() << ", message " << message;

    auto request_args = websocket::serializeTransaction(tx);
    web_socket.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


void push_contract(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    std::string keys_dir_str{ arguments[0] };
    std::string fee_str{ arguments[1] };
    std::string amount_str{ arguments[2] };
    std::string path_to_compiled_folder{ arguments[3] };
    std::string message{ arguments[4] };
    auto& web_socket{ client.getWebSocket() };

    auto amount = takeAmount(client, amount_str);
    if (!amount) {
        return;
    }

    auto fee = takeFee(client, fee_str);
    if (!fee) {
        return;
    }

    auto data = takeMessage(client, message);
    if (!data) {
        return;
    }

    auto wallets = client.getWallets();

    std::filesystem::path keys_dir = wallets.find(keys_dir_str) != wallets.end() ? wallets[keys_dir_str] : keys_dir_str;

    auto private_key_path = base::config::makePrivateKeyPath(keys_dir);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(lk::Address::null());
    txb.setAmount(amount.value());
    txb.setTimestamp(base::Time::now());
    txb.setFee(fee.value());

    if (data.value().isEmpty()) {
        auto code_binary_file_path = path_to_compiled_folder / std::filesystem::path(config::CONTRACT_BINARY_FILE);
        if (!std::filesystem::exists(code_binary_file_path)) {
            client.output("Error the file with this path[" + code_binary_file_path.string() + "] does not exist");
            return;
        }
        std::ifstream file(code_binary_file_path, std::ios::binary);
        auto buf = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        data = base::fromHex<base::Bytes>(buf);
    }
    txb.setData(data.value());

    auto tx = std::move(txb).build();

    tx.sign(private_key);

    auto tx_hash = tx.hashOfTransaction();
    client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Push_contract from " << from_address << ", amount " << amount.value() << ", fee " << fee.value()
             << ", message " << message;

    auto request_args = websocket::serializeTransaction(tx);
    web_socket.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


void subscribe_last_block_info(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    auto& web_socket{ client.getWebSocket() };
    LOG_INFO << "subscription last_block_info";
    web_socket.send(websocket::Command::SUBSCRIBE_LAST_BLOCK_INFO, base::json::Value::object());
}


void unsubscribe_last_block_info(Client& client, const std::vector<std::string>& arguments)
{
    if (!client.isConnected()) {
        client.output("You have to connect to likelib node");
        return;
    }

    auto& web_socket{ client.getWebSocket() };
    LOG_INFO << "unsubscription last_block_info";
    web_socket.send(websocket::Command::UNSUBSCRIBE_LAST_BLOCK_INFO, base::json::Value::object());
}
