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

void help(Client& client)
{
    std::string output = "- help\n"
                         "      Show help message\n"
                         "- connect <ip and port of likelib node>\n"
                         "      Connect client to specific likelib node\n"
                         "- disconnect\n"
                         "      Disconnect client from likelib node\n"
                         "- compile <path to solidity code file>\n"
                         "      Compile solidity code to binary format\n"
                         "- encode <path to folder with compiled contract data files> <message for encode>\n"
                         "      Encode message for contract call\n"
                         "- decode <path to folder with compiled contract data files> <message for decode>\n"
                         "      Decode message which was returned by contract call\n"
                         "- keys_generate <path to folder to save key>\n"
                         "      Generate new key and store to specific folder\n"
                         "- keys_info <path to folder with key\n"
                         "      Print info about specified key\n"
                         "- last_block_info\n"
                         "      Get last block info\n"
                         "- account_info <address of base58>\n"
                         "      Get account info by specific address\n"
                         "- subscribe_account_info <address of base58>\n"
                         "      Subscribe on updates account info by specific address\n"
                         "- unsubscribe_account_info <address at base58>\n"
                         "      Unsubscribe from account info updates by specific address\n"
                         "- subscribe_last_block_info\n"
                         "      Get last block info when apeared new block\n"
                         "- unsubscribe_last_block_info\n"
                         "      Get last block info when apeared new block\n"
                         "- find_transaction <transaction hash at hex>\n"
                         "      Get transaction by specific hash\n"
                         "- find_transaction_status <transaction hash at hex>\n"
                         "      Get transaction status by specific hash\n"
                         "- find_block <block hash at hex>\n"
                         "      Get block by specific hash\n"
                         "- transfer <path to folder with key> <address of recipient at base58>"
                         " <fee for transaction> <amount of coins for transfer>\n"
                         "      Transfer coins to specific address\n"
                         "- contract_call <path to folder with key> <address of contract at base58> "
                         "<fee for contract call> <amount of coins for call> <message for call at hex>\n"
                         "      Call deployed contract\n"
                         "- push_contract <path to folder with key> <fee for contract call> <amount of coins for call>"
                         "<path to compiled contract data files> <message for initializing contract at hex>\n"
                         "      Deploy compiled contract\n";
    client.output(output);
}


void compile_solidity_code(Client& client, const std::string& code_file_path)
{
    std::optional<vm::Contracts> contracts;
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


void encode_message(Client& client, const std::string& compiled_contract_folder_path, const std::string& message)
{
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


void decode_message(Client& client, const std::string& compiled_contract_folder_path, const std::string& message)
{
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


void generate_keys(Client& client, const std::string& path)
{
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


void keys_info(Client& client, const std::string& path)
{
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


void call_last_block_info(websocket::WebSocketClient& client)
{
    LOG_INFO << "last_block_info";
    client.send(websocket::Command::CALL_LAST_BLOCK_INFO, base::json::Value::object());
}


void call_account_info(websocket::WebSocketClient& client, const lk::Address& address)
{
    LOG_INFO << "account_info for address: " << address;
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(address);
    client.send(websocket::Command::CALL_ACCOUNT_INFO, std::move(request_args));
}


void subscribe_account_info(websocket::WebSocketClient& client, const lk::Address& address)
{
    LOG_INFO << "account_info for address: " << address;
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(address);
    client.send(websocket::Command::SUBSCRIBE_ACCOUNT_INFO, std::move(request_args));
}


void unsubscribe_account_info(websocket::WebSocketClient& client, const lk::Address& address)
{
    LOG_INFO << "account_info for address: " << address;
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(address);
    client.send(websocket::Command::UNSUBSCRIBE_ACCOUNT_INFO, std::move(request_args));
}


void call_find_transaction(websocket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_transaction by hash: " << hash;
    auto request_args = base::json::Value::object();
    request_args["rapidjson"] = websocket::serializeHash(hash);
    client.send(websocket::Command::CALL_FIND_TRANSACTION, std::move(request_args));
}


void call_find_transaction_status(websocket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_transaction_status by hash: " << hash;
    auto request_args = base::json::Value::object();
    request_args["rapidjson"] = websocket::serializeHash(hash);
    client.send(websocket::Command::CALL_FIND_TRANSACTION_STATUS, std::move(request_args));
}


void call_find_block(websocket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_block by hash: " << hash;
    auto request_args = base::json::Value::object();
    request_args["rapidjson"] = websocket::serializeHash(hash);
    client.send(websocket::Command::CALL_FIND_BLOCK, std::move(request_args));
}


void transfer(Client& client,
              websocket::WebSocketClient& web_socket,
              const lk::Address& to_address,
              const lk::Balance& amount,
              const lk::Fee& fee,
              const std::filesystem::path& keys_dir)
{
    auto private_key_path = base::config::makePrivateKeyPath(keys_dir);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(to_address);
    txb.setAmount(amount);
    txb.setTimestamp(base::Time::now());
    txb.setFee(fee);
    txb.setData({});
    auto tx = std::move(txb).build();

    tx.sign(private_key);

    auto tx_hash = tx.hashOfTransaction();
    client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Transfer from " << from_address << " to " << to_address << " with amount " << amount;

    auto request_args = websocket::serializeTransaction(tx);
    web_socket.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


void contract_call(Client& client,
                   websocket::WebSocketClient& web_socket,
                   const lk::Address& to_address,
                   const lk::Balance& amount,
                   const lk::Fee& fee,
                   const std::filesystem::path& keys_dir,
                   const std::string& message)
{
    auto private_key_path = base::config::makePrivateKeyPath(keys_dir);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(to_address);
    txb.setAmount(amount);
    txb.setTimestamp(base::Time::now());
    txb.setFee(fee);
    txb.setData(base::fromHex<base::Bytes>(message));
    auto tx = std::move(txb).build();

    tx.sign(private_key);

    auto tx_hash = tx.hashOfTransaction();
    client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Contract_call from " << from_address << ", to " << to_address << ", amount " << amount << ",fee "
             << fee << ", message " << message;

    auto request_args = websocket::serializeTransaction(tx);
    web_socket.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


void push_contract(Client& client,
                   websocket::WebSocketClient& web_socket,
                   const lk::Balance& amount,
                   const lk::Fee& fee,
                   const std::filesystem::path& keys_dir,
                   const std::filesystem::path& path_to_compiled_folder,
                   const std::string& message)
{
    auto private_key_path = base::config::makePrivateKeyPath(keys_dir);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(lk::Address::null());
    txb.setAmount(amount);
    txb.setTimestamp(base::Time::now());
    txb.setFee(fee);

    base::Bytes data = base::fromHex<base::Bytes>(message);
    if (data.isEmpty()) {
        auto code_binary_file_path = path_to_compiled_folder / std::filesystem::path(config::CONTRACT_BINARY_FILE);
        if (!std::filesystem::exists(code_binary_file_path)) {
            client.output("Error the file with this path[" + code_binary_file_path.string() + "] does not exist");
            return;
        }
        std::ifstream file(code_binary_file_path, std::ios::binary);
        auto buf = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        data = base::fromHex<base::Bytes>(buf);
    }
    txb.setData(data);

    auto tx = std::move(txb).build();

    tx.sign(private_key);

    auto tx_hash = tx.hashOfTransaction();
    client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Push_contract from " << from_address << ", amount " << amount << ", fee " << fee << ", message "
             << message;

    auto request_args = websocket::serializeTransaction(tx);
    web_socket.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


void subscribe_last_block_info(websocket::WebSocketClient& client)
{
    LOG_INFO << "subscription last_block_info";
    client.send(websocket::Command::SUBSCRIBE_LAST_BLOCK_INFO, base::json::Value::object());
}


void unsubscribe_last_block_info(websocket::WebSocketClient& client)
{
    LOG_INFO << "unsubscription last_block_info";
    client.send(websocket::Command::UNSUBSCRIBE_LAST_BLOCK_INFO, base::json::Value::object());
}
