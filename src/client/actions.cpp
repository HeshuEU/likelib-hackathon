#include "actions.hpp"

#include "client/config.hpp"

#include "core/transaction.hpp"

#include "web_socket/error.hpp"
#include "web_socket/tools.hpp"

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

std::uint64_t last_id{ 0 };


base::PropertyTree generate_query(const std::string& type, const std::string& command_name, base::PropertyTree& args)
{
    base::PropertyTree query;
    query.add("type", type);
    query.add("name", command_name);
    query.add("api", base::config::RPC_PUBLIC_API_VERSION);
    query.add("id", ++last_id);
    query.add("args", args);

    return query;
}


base::PropertyTree generate_call(const std::string& command_name, base::PropertyTree& args)
{
    return generate_query("call", command_name, args);
}

base::PropertyTree generate_subscription(const std::string& command_name, base::PropertyTree& args)
{
    return generate_query("subscribe", command_name, args);
}


void compile_solidity_code(std::ostream& output, const std::string& code_file_path)
{
    std::optional<vm::Contracts> contracts;
    try {
        contracts = vm::compile(code_file_path);
    }
    catch (const base::ParsingError& er) {
        output << er;
        return;
    }
    catch (const base::SystemCallFailed& er) {
        output << er;
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
            output << er;
            LOG_ERROR << er;
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
        output << er;
        return;
    }
    catch (const base::SystemCallFailed& er) {
        output << er;
        return;
    }
}


void decode_message(std::ostream& output,
                    const std::string& compiled_contract_folder_path,
                    const std::string& method,
                    const std::string& message)
{
    try {
        auto output_message = vm::decodeOutput(compiled_contract_folder_path, method, message);
        if (output_message) {
            output << output_message.value() << std::endl;
        }
        else {
            output << "decoding failed.\n";
            return;
        }
    }
    catch (const base::ParsingError& er) {
        output << er;
        return;
    }
    catch (const base::SystemCallFailed& er) {
        output << er;
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


void call_last_block_info(web_socket::WebSocketClient& client)
{
    LOG_INFO << "last_block_info";
    base::PropertyTree null;
    auto request = generate_call("last_block_info", null);
    client.send(request);
}


void call_account_info(web_socket::WebSocketClient& client, const lk::Address& address)
{
    LOG_INFO << "account_info for address: " << address;
    base::PropertyTree request_args;
    request_args.add("address", web_socket::serializeAddress(address));
    auto request = generate_call("account_info", request_args);
    client.send(request);
}


void call_find_transaction(web_socket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_transaction by hash: " << hash;
    base::PropertyTree request_args;
    request_args.add("hash", web_socket::serializeHash(hash));
    auto request = generate_call("find_transaction", request_args);
    client.send(request);
}


void call_find_transaction_status(web_socket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_transaction_status by hash: " << hash;
    base::PropertyTree request_args;
    request_args.add("hash", web_socket::serializeHash(hash));
    auto request = generate_call("find_transaction_status", request_args);
    client.send(request);
}


void call_find_block(web_socket::WebSocketClient& client, const base::Sha256& hash)
{
    LOG_INFO << "find_block by hash: " << hash;
    base::PropertyTree request_args;
    request_args.add("hash", web_socket::serializeHash(hash));
    auto request = generate_call("find_block", request_args);
    client.send(request);
}


void call_contract_view([[maybe_unused]] std::ostream& output,
                        web_socket::WebSocketClient& client,
                        const lk::Address& to_address,
                        const std::filesystem::path& keys_dir,
                        const std::string& message)
{
    auto private_key_path = base::config::makePrivateKeyPath(keys_dir);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    auto data = base::fromHex<base::Bytes>(message);
    lk::ViewCall call{ from_address, to_address, base::Time::now(), std::move(data) };
    call.sign(private_key);

    LOG_INFO << "Contract view call from " << from_address << " to " << to_address << " message " << message;
    auto request_args = web_socket::serializeViewCall(call);
    auto request = generate_call("call_contract_view", request_args);
    client.send(request);
}


void transfer(std::ostream& output,
              web_socket::WebSocketClient& client,
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

    base::PropertyTree request_args = web_socket::serializeTransaction(tx);
    auto request = generate_subscription("push_transaction", request_args);
    client.send(request);
}


void contract_call(std::ostream& output,
                   web_socket::WebSocketClient& client,
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

    base::PropertyTree request_args = web_socket::serializeTransaction(tx);
    auto request = generate_subscription("push_transaction", request_args);
    client.send(request);
}


void push_contract(std::ostream& output,
                   web_socket::WebSocketClient& client,
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

    base::PropertyTree request_args = web_socket::serializeTransaction(tx);
    auto request = generate_subscription("push_transaction", request_args);
    client.send(request);
}


void subscribe_last_block_info(web_socket::WebSocketClient& client)
{
    LOG_INFO << "subscription last_block_info";
    base::PropertyTree null;
    auto request = generate_subscription("last_block_info", null);
    client.send(request);
}


void subscribe_account_info(web_socket::WebSocketClient& client, const lk::Address& address)
{
    LOG_INFO << "subscription account_info for address: " << address;
    base::PropertyTree request_args;
    request_args.add("address", web_socket::serializeAddress(address));
    auto request = generate_call("account_info", request_args);
    client.send(request);
}
