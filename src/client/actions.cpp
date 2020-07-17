#include "actions.hpp"

#include "client/config.hpp"

#include "core/transaction.hpp"

#include "websocket/error.hpp"
#include "websocket/tools.hpp"

#include "vm/tools.hpp"
#include "vm/vm.hpp"

#include "base/config.hpp"
#include "base/directory.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"
#include "base/property_tree.hpp"
#include "base/time.hpp"

#include <cstring>
#include <iostream>
#include <string>

void compile_solidity_code(std::ostream& output, const std::string& code_file_path)
{
    std::optional<vm::Contracts> contracts;
    try {
        contracts = vm::compile(code_file_path);
    }
    catch (const base::ParsingError& er) {
        output << er.what();
        return;
    }
    catch (const base::SystemCallFailed& er) {
        output << er.what();
        return;
    }

    if (!contracts) {
        output << "Compilation error\n";
        return;
    }

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
                file.close();
            }

            base::save(contract.metadata, current_folder / std::filesystem::path{ config::METADATA_JSON_FILE });
        }
        catch (const base::Error& er) {
            output << er.what();
            LOG_ERROR << er.what();
            return;
        }
        catch (...) {
            output << "unexpected error at saving contract:" << contract.name;
            LOG_ERROR << "unexpected error at saving contract:" << contract.name;
            return;
        }
    }
}


void encode_message(std::ostream& output, const std::string& compiled_contract_folder_path, const std::string& message)
{
    try {
        auto output_message = vm::encodeCall(compiled_contract_folder_path, message);
        if (output_message) {
            output << output_message.value() << std::endl;
        }
        else {
            output << "encoding failed.\n";
            return;
        }
    }
    catch (const base::ParsingError& er) {
        output << er.what();
        return;
    }
    catch (const base::SystemCallFailed& er) {
        output << er.what();
        return;
    }
}


void decode_message(std::ostream& output, const std::string& compiled_contract_folder_path, const std::string& message)
{
    try {
        auto output_message = vm::decodeOutput(compiled_contract_folder_path, message);
        if (output_message) {
            output << output_message.value() << std::endl;
        }
        else {
            output << "decoding failed.\n";
            return;
        }
    }
    catch (const base::ParsingError& er) {
        output << er.what();
        return;
    }
    catch (const base::SystemCallFailed& er) {
        output << er.what();
        return;
    }
}


void generate_keys(std::ostream& output, const std::string& path)
{
    const auto& priv = base::Secp256PrivateKey();

    auto private_path = base::config::makePrivateKeyPath(path);
    if (std::filesystem::exists(private_path)) {
        output << "Error: " << private_path << " already exists.\n";
        LOG_ERROR << private_path << " file already exists";
        return;
    }

    priv.save(private_path);

    output << "Generated key at " << path << std::endl;
    output << "Address: " << lk::Address(priv.toPublicKey()) << std::endl;
    output << "Hash of public key: " << base::Sha256::compute(priv.toPublicKey().toBytes()) << std::endl;
    output << "Hash of private key: " << base::Sha256::compute(priv.getBytes().toBytes()) << std::endl;
    LOG_INFO << "Generated key at " << path;

    priv.save(private_path);
}


void keys_info(std::ostream& output, const std::string& path)
{
    auto private_path = base::config::makePrivateKeyPath(path);
    if (!std::filesystem::exists(private_path)) {
        output << "Error: " << private_path << " doesn't exist.\n";
        LOG_ERROR << private_path << " file not exists";
        return;
    }

    auto priv = base::Secp256PrivateKey::load(private_path);

    output << "Address: " << lk::Address(priv.toPublicKey()) << std::endl;
    output << "Hash of public key: " << base::Sha256::compute(priv.toPublicKey()) << std::endl;
    output << "Hash of private key: " << base::Sha256::compute(priv.getBytes()) << std::endl;
}


void call_last_block_info(websocket::WebSocketClient& client)
{
    LOG_INFO << "last_block_info";
    client.send(websocket::Command::CALL_LAST_BLOCK_INFO, base::PropertyTree{});
}


void call_account_info(websocket::WebSocketClient& client, const lk::Address& address)
{
    LOG_INFO << "account_info for address: " << address;
    base::PropertyTree request_args;
    request_args.add("address", websocket::serializeAddress(address));
    client.send(websocket::Command::CALL_ACCOUNT_INFO, request_args);
}


void subscribe_account_info(websocket::WebSocketClient& client, const lk::Address& address)
{
    LOG_INFO << "account_info for address: " << address;
    base::PropertyTree request_args;
    request_args.add("address", websocket::serializeAddress(address));
    client.send(websocket::Command::SUBSCRIBE_ACCOUNT_INFO, request_args);
}


void unsubscribe_account_info(websocket::WebSocketClient& client, const lk::Address& address)
{
    LOG_INFO << "account_info for address: " << address;
    base::PropertyTree request_args;
    request_args.add("address", websocket::serializeAddress(address));
    client.send(websocket::Command::UNSUBSCRIBE_ACCOUNT_INFO, request_args);
}


void call_find_transaction(websocket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_transaction by hash: " << hash;
    base::PropertyTree request_args;
    request_args.add("hash", websocket::serializeHash(hash));
    client.send(websocket::Command::CALL_FIND_TRANSACTION, request_args);
}


void call_find_transaction_status(websocket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_transaction_status by hash: " << hash;
    base::PropertyTree request_args;
    request_args.add("hash", websocket::serializeHash(hash));
    client.send(websocket::Command::CALL_FIND_TRANSACTION_STATUS, request_args);
}


void call_find_block(websocket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_block by hash: " << hash;
    base::PropertyTree request_args;
    request_args.add("hash", websocket::serializeHash(hash));
    client.send(websocket::Command::CALL_FIND_BLOCK, request_args);
}


void transfer(std::ostream& output,
              websocket::WebSocketClient& client,
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
    output << "Transaction with hash[hex]: " << tx_hash << std::endl;

    LOG_INFO << "Transfer from " << from_address << " to " << to_address << " with amount " << amount;

    base::PropertyTree request_args = websocket::serializeTransaction(tx);
    client.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, request_args);
}


void contract_call(std::ostream& output,
                   websocket::WebSocketClient& client,
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
    output << "Transaction with hash[hex]: " << tx_hash << std::endl;

    LOG_INFO << "Contract_call from " << from_address << ", to " << to_address << ", amount " << amount << ",fee "
             << fee << ", message " << message;

    base::PropertyTree request_args = websocket::serializeTransaction(tx);
    client.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, request_args);
}


void push_contract(std::ostream& output,
                   websocket::WebSocketClient& client,
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
            output << "Error the file with this path[" + code_binary_file_path.string() + "] does not exist";
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
    output << "Transaction with hash[hex]: " << tx_hash << std::endl;

    LOG_INFO << "Push_contract from " << from_address << ", amount " << amount << ", fee " << fee << ", message "
             << message;

    base::PropertyTree request_args = websocket::serializeTransaction(tx);
    client.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, request_args);
}


void subscribe_last_block_info(websocket::WebSocketClient& client)
{
    LOG_INFO << "subscription last_block_info";
    client.send(websocket::Command::SUBSCRIBE_LAST_BLOCK_INFO, base::PropertyTree{});
}


void unsubscribe_last_block_info(websocket::WebSocketClient& client)
{
    LOG_INFO << "unsubscription last_block_info";
    client.send(websocket::Command::UNSUBSCRIBE_LAST_BLOCK_INFO, base::PropertyTree{});
}

