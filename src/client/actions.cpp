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

void clearSpaces(std::string& str)
{
    std::size_t count_spaces{ 0 };
    while (count_spaces < str.size() && (str[count_spaces] == ' ' || str[count_spaces] == '\t')) {
        count_spaces++;
    }
    if (count_spaces >= str.size()) {
        str.clear();
    }
    else {
        str.erase(0, count_spaces);
    }
}

std::string parseArgument(std::string& input)
{
    clearSpaces(input);
    if (input.empty()) {
        return input;
    }
    char quote;
    bool is_text{ false };
    if ((input[0] == '\'') || (input[0] == '\"')) {
        quote = input[0];
        is_text = true;
        input.erase(0, 1);
    }
    std::size_t end{ 0 };

    while ((end < input.size()) && (is_text || (input[end] != ' '))) {
        if (!is_text && ((input[end] == '\'') || (input[end] == '\"'))) {
            is_text = true;
            quote = input[end];
            input.erase(end, 1);
        }
        else if (!is_text && (input[end] == '\\')) {
            input.erase(end, 1);
            if (input.size() <= end) {
                return std::string{};
            }
            else {
                end++;
            }
        }
        else if (is_text && input[end] == quote) {
            is_text = false;
            input.erase(end, 1);
        }
        else {
            end++;
        }
    }
    if (is_text) {
        return std::string{};
    }
    auto argument = input.substr(0, end);
    input.erase(0, end);
    return argument;
}


std::vector<std::string> parseAllArguments(std::string& input)
{
    std::vector<std::string> arguments;
    while (!input.empty()) {
        auto argument = parseArgument(input);
        if (!argument.empty()) {
            arguments.push_back(argument);
        }
    }
    return arguments;
}

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


std::optional<lk::Balance> takeAmount(Client& client, std::string amount_str)
{
    lk::Balance amount;
    bool is_token{ false };
    if (amount_str.find('.') != std::string::npos) {
        is_token = true;
        // amount_str.erase(amount_str.size() - std::strlen(base::config::BC_TOKEN_NAME));
    }
    try {
        if (is_token) {
            std::size_t lkl_size = std::to_string(base::config::BC_TOKEN_VALUE).size() - 1;
            std::size_t numbers_size_after_dot = amount_str.size() - amount_str.find('.') - 1;
            for (numbers_size_after_dot; numbers_size_after_dot < lkl_size; numbers_size_after_dot++) {
                amount_str += '0';
            }
            amount_str.erase(amount_str.find('.'), 1);

            while (amount_str[0] == '0') {
                amount_str.erase(0, 1);
            }
            amount = lk::Balance{ amount_str };
        }
        else {
            client.output("Wrong amount was entered");
            return {};
        }
    }
    catch (const boost::wrapexcept<std::runtime_error>& er) {
        client.output("Wrong amount was entered");
        return {};
    }
    return amount;
}


std::optional<lk::BlockDepth> takeDepth(Client& client, const std::string depth_str)
{
    lk::BlockDepth depth;
    try {
        depth = std::stoull(depth_str);
    }
    catch (const boost::wrapexcept<std::runtime_error>& er) {
        client.output("Wrong depth was entered");
        return {};
    }
    return depth;
}


std::optional<base::Sha256> takeHash(Client& client, const std::string hash, bool continue_work = false)
{
    base::FixedBytes<base::Sha256::LENGTH> hash_bytes;
    try {
        hash_bytes = base::FixedBytes<base::Sha256::LENGTH>(base::fromHex<base::Bytes>(hash));
    }
    catch (const base::InvalidArgument& er) {
        if (!continue_work) {
            client.output("Wrong hash was entered");
        }
        return {};
    }
    return base::Sha256{ hash_bytes };
}


Command::Command(Client& client, std::size_t count_arguments)
  : _client(client)
  , _count_arguments{ count_arguments }
{}

void Command::run(const std::string& arguments) // TODO: change catches
{
    _args = arguments;
    bool is_ok{ false };
    try {
        is_ok = prepareArgs();
    }
    catch (...) {
        LOG_ERROR << name() << "| unexpected error during prepare arguments for execution CLI command";
    }

    try {
        if (is_ok) {
            execute();
        }
    }
    catch (const base::Error& error) {
        LOG_DEBUG << name() << "execution CLI command was failed: ";
    }
    catch (...) {
        LOG_ERROR << name() << "| unexpected error during execution CLI command";
    }
}


std::optional<std::string> Command::completionGenerator(const std::string& input)
{
    return {};
}


std::size_t Command::getCountArguments()
{
    return _count_arguments;
}


HelpCommand::HelpCommand(Client& client)
  : Command{ client, 0 }
{}


const std::string& HelpCommand::name() const noexcept
{
    static std::string name{ "help" };
    return name;
}


const std::string& HelpCommand::description() const noexcept
{
    static std::string description{ "Show help message" };
    return description;
}


const std::string& HelpCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool HelpCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    return true;
}


void HelpCommand::execute()
{
    for (const auto& command : _client._commands) {
        _client.output("- " + command->name() + " " + command->argumentsHelpMessage() + "\n\t\t" +
                       command->description() + "\n");
    }
}


ConnectCommand::ConnectCommand(Client& client)
  : Command{ client, 1 }
{}


const std::string& ConnectCommand::name() const noexcept
{
    static std::string name{ "connect" };
    return name;
}


const std::string& ConnectCommand::description() const noexcept
{
    static std::string description{ "Connect client to specific likelib node" };
    return description;
}


const std::string& ConnectCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<ip and port of likelib node>" };
    return help_message;
}


bool ConnectCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _host = arguments[0];
    return true;
}


void ConnectCommand::execute()
{
    _client._connected = _client._web_socket_client.connect(_host);
    if (_client._connected) {
        _client._host = _host;
        _client._thread = std::thread{ [&]() {
            if (_client._io_context.stopped()) {
                _client._io_context.restart();
            }
            _client._io_context.run();
        } };
        _client.output("Client connected to host " + _host);
    }
    else {
        _client.output("Can't connect to host " + _host);
    }
}


DisconnectCommand::DisconnectCommand(Client& client)
  : Command{ client, 0 }
{}


const std::string& DisconnectCommand::name() const noexcept
{
    static std::string name{ "disconnect" };
    return name;
}


const std::string& DisconnectCommand::description() const noexcept
{
    static std::string description{ "Disconnect client from likelib node" };
    return description;
}


const std::string& DisconnectCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool DisconnectCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    return true;
}


void DisconnectCommand::execute()
{
    _client._connected = false;
    _client._io_context.stop();
    _client._web_socket_client.disconnect();
    _client._thread.join();
}


ExitCommand::ExitCommand(Client& client)
  : Command{ client, 0 }
{}


const std::string& ExitCommand::name() const noexcept
{
    static std::string name{ "exit" };
    return name;
}


const std::string& ExitCommand::description() const noexcept
{
    static std::string description{ "Exit fron likelib client" };
    return description;
}


const std::string& ExitCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool ExitCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    return true;
}


void ExitCommand::execute()
{
    _client._io_context.stop();
    _client._web_socket_client.disconnect();
    _client._exit = true;
}


CompilyCommand::CompilyCommand(Client& client)
  : Command{ client, 1 }
{}


const std::string& CompilyCommand::name() const noexcept
{
    static std::string name{ "compile" };
    return name;
}


const std::string& CompilyCommand::description() const noexcept
{
    static std::string description{ "Compile solidity code to binary format" };
    return description;
}


const std::string& CompilyCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<path to solidity code file>" };
    return help_message;
}


bool CompilyCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _code_path = arguments[0];
    return true;
}


void CompilyCommand::execute()
{
    std::optional<vm::Contracts> contracts;
    try {
        contracts = vm::compile(_code_path);
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
            _client.output(output.str());
            LOG_ERROR << er.what();
            return;
        }
        catch (...) {
            _client.output(output.str());
            LOG_ERROR << "unexpected error at saving contract:" << contract.name;
            return;
        }
        _client.output(output.str());
    }
}


EncodeCommand::EncodeCommand(Client& client)
  : Command(client, 2)
{}


const std::string& EncodeCommand::name() const noexcept
{
    static std::string name{ "encode" };
    return name;
}


const std::string& EncodeCommand::description() const noexcept
{
    static std::string description{ "Encode message for contract call" };
    return description;
}


const std::string& EncodeCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<path to folder with compiled contract data files> <message for encode>" };
    return help_message;
}


bool EncodeCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _compiled_contract_folder_path = arguments[0];
    _message = arguments[1];
    return true;
}


void EncodeCommand::execute()
{
    try {
        auto output_message = vm::encodeCall(_compiled_contract_folder_path, _message);
        if (output_message) {
            _client.output(base::toHex(output_message.value()));
        }
        else {
            _client.output("encoding failed.\n");
            return;
        }
    }
    catch (const base::ParsingError& er) {
        _client.output(er.what());
        return;
    }
    catch (const base::SystemCallFailed& er) {
        _client.output(er.what());
        return;
    }
}


DecodeCommand::DecodeCommand(Client& client)
  : Command(client, 2)
{}


const std::string& DecodeCommand::name() const noexcept
{
    static std::string name{ "decode" };
    return name;
}


const std::string& DecodeCommand::description() const noexcept
{
    static std::string description{ "Decode message for contract call" };
    return description;
}


const std::string& DecodeCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<path to folder with compiled contract data files> <message for decode>" };
    return help_message;
}


bool DecodeCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _compiled_contract_folder_path = arguments[0];
    _message = arguments[1];
    return true;
}


void DecodeCommand::execute()
{
    try {
        auto output_message = vm::decodeOutput(_compiled_contract_folder_path, _message);
        if (output_message) {
            _client.output(output_message.value());
        }
        else {
            _client.output("decoding failed.\n");
            return;
        }
    }
    catch (const base::ParsingError& er) {
        _client.output(er.what());
        return;
    }
    catch (const base::SystemCallFailed& er) {
        _client.output(er.what());
        return;
    }
}


KeysGenerateCommand::KeysGenerateCommand(Client& client)
  : Command(client, 1)
{}


const std::string& KeysGenerateCommand::name() const noexcept
{
    static std::string name{ "keys_generate" };
    return name;
}


const std::string& KeysGenerateCommand::description() const noexcept
{
    static std::string description{ "Generate new key and store to specific folder" };
    return description;
}


const std::string& KeysGenerateCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<path to folder to save key>" };
    return help_message;
}


bool KeysGenerateCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _keys_path = arguments[0];
    return true;
}


void KeysGenerateCommand::execute()
{
    const auto& priv = base::Secp256PrivateKey();

    auto private_path = base::config::makePrivateKeyPath(_keys_path);
    std::stringstream output;
    if (std::filesystem::exists(private_path)) {
        output << "Error: " << private_path << " already exists.\n";
        _client.output(output.str());
        LOG_ERROR << private_path << " file already exists";
        return;
    }

    priv.save(private_path);

    output << "Generated key at " << _keys_path << std::endl;
    output << "Address: " << lk::Address(priv.toPublicKey()) << std::endl;
    output << "Hash of public key: " << base::Sha256::compute(priv.toPublicKey().toBytes()) << std::endl;
    output << "Hash of private key: " << base::Sha256::compute(priv.getBytes().toBytes()) << std::endl;
    _client.output(output.str());
    LOG_INFO << "Generated key at " << _keys_path;

    priv.save(private_path);
}


KeysInfoCommand::KeysInfoCommand(Client& client)
  : Command(client, 1)
{}


const std::string& KeysInfoCommand::name() const noexcept
{
    static std::string name{ "keys_info" };
    return name;
}


const std::string& KeysInfoCommand::description() const noexcept
{
    static std::string description{ "Print info about specified key" };
    return description;
}


const std::string& KeysInfoCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<path to folder with key>" };
    return help_message;
}


bool KeysInfoCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _keys_path = arguments[0];
    return true;
}


void KeysInfoCommand::execute()
{
    auto private_path = base::config::makePrivateKeyPath(_keys_path);
    std::stringstream output;
    if (!std::filesystem::exists(private_path)) {
        output << "Error: " << private_path << " doesn't exist.\n";
        _client.output(output.str());
        LOG_ERROR << private_path << " file not exists";
        return;
    }

    auto priv = base::Secp256PrivateKey::load(private_path);

    output << "Address: " << lk::Address(priv.toPublicKey()) << std::endl;
    output << "Hash of public key: " << base::Sha256::compute(priv.toPublicKey()) << std::endl;
    output << "Hash of private key: " << base::Sha256::compute(priv.getBytes()) << std::endl;
    _client.output(output.str());
}


AddContactCommand::AddContactCommand(Client& client)
  : Command(client, 2)
{}


const std::string& AddContactCommand::name() const noexcept
{
    static std::string name{ "add_contact" };
    return name;
}


const std::string& AddContactCommand::description() const noexcept
{
    static std::string description{ "Saves the address under the selected name" };
    return description;
}


const std::string& AddContactCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<contact name> <contact address>" };
    return help_message;
}


bool AddContactCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _contact_name = arguments[0];
    _address = takeAddress(_client, arguments[1]);
    if (!_address) {
        return false;
    }
    return true;
}


void AddContactCommand::execute()
{
    auto& contacts = _client.getContacts();
    if (contacts.find(_contact_name) == contacts.end()) {
        _client.addContact(_contact_name, _address);

        _client.output("Added a contact with the name " + _contact_name);
    }
    else {
        _client.output("Contact with the name " + _contact_name + " already exists");
    }
}


DeleteContactCommand::DeleteContactCommand(Client& client)
  : Command(client, 1)
{}


const std::string& DeleteContactCommand::name() const noexcept
{
    static std::string name{ "delete_contact" };
    return name;
}


const std::string& DeleteContactCommand::description() const noexcept
{
    static std::string description{ "Delete a contact with the selected name" };
    return description;
}


const std::string& DeleteContactCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<contact name>" };
    return help_message;
}


bool DeleteContactCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _contact_name = arguments[0];
    return true;
}


void DeleteContactCommand::execute()
{
    auto& contacts = _client.getContacts();

    if (contacts.find(_contact_name) == contacts.end()) {
        _client.output("Contact with the name " + _contact_name + " does not exists");
    }
    else {
        _client.deleteContact(_contact_name);
        _client.output("Contact with the name " + _contact_name + " deleted");
    }
}


ShowContactsCommand::ShowContactsCommand(Client& client)
  : Command(client, 0)
{}


const std::string& ShowContactsCommand::name() const noexcept
{
    static std::string name{ "show_contacts" };
    return name;
}


const std::string& ShowContactsCommand::description() const noexcept
{
    static std::string description{ "Displays all saved contacts" };
    return description;
}


const std::string& ShowContactsCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool ShowContactsCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    return true;
}


void ShowContactsCommand::execute()
{
    auto& contacts = _client.getContacts();

    if (contacts.empty()) {
        _client.output("no contacts saved");
    }
    else {
        for (const auto& contact : contacts) {
            _client.output(contact.first + " - " + contact.second.value().toString());
        }
    }
}


AddWalletCommand::AddWalletCommand(Client& client)
  : Command(client, 2)
{}


const std::string& AddWalletCommand::name() const noexcept
{
    static std::string name{ "add_wallet" };
    return name;
}


const std::string& AddWalletCommand::description() const noexcept
{
    static std::string description{ "Remembers the path to the key, under the selected name" };
    return description;
}


const std::string& AddWalletCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<name of new wallet> <path to foldef with key>" };
    return help_message;
}


bool AddWalletCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _wallet_name = arguments[0];
    _keys_path = arguments[1];
    return true;
}


void AddWalletCommand::execute()
{
    auto wallets = _client.getWallets();

    auto private_key_path = base::config::makePrivateKeyPath(_keys_path);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    if (wallets.find(_wallet_name) == wallets.end()) {
        _client.addWallet(_wallet_name, _keys_path);
        _client.output("Added a wallet with the name " + _wallet_name);
    }
    else {
        _client.output("Wallet with the name " + _wallet_name + " already exists");
    }
}


DeleteWalletCommand::DeleteWalletCommand(Client& client)
  : Command(client, 1)
{}


const std::string& DeleteWalletCommand::name() const noexcept
{
    static std::string name{ "delete_wallet" };
    return name;
}


const std::string& DeleteWalletCommand::description() const noexcept
{
    static std::string description{ "Deletes the wallet with the selected name" };
    return description;
}


const std::string& DeleteWalletCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<wallet name to delete>" };
    return help_message;
}


bool DeleteWalletCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _wallet_name = arguments[0];
    return true;
}


void DeleteWalletCommand::execute()
{
    auto wallets = _client.getWallets();

    if (wallets.find(_wallet_name) == wallets.end()) {
        _client.output("Wallet with the name " + _wallet_name + " does not exists");
    }
    else {
        _client.deleteWallet(_wallet_name);
        _client.output("Wallet with the name " + _wallet_name + " deleted");
    }
}


ShowWalletsCommand::ShowWalletsCommand(Client& client)
  : Command(client, 0)
{}


const std::string& ShowWalletsCommand::name() const noexcept
{
    static std::string name{ "show_wallets" };
    return name;
}


const std::string& ShowWalletsCommand::description() const noexcept
{
    static std::string description{ "Outputs all memorized wallet names and their corresponding paths" };
    return description;
}


const std::string& ShowWalletsCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool ShowWalletsCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    return true;
}


void ShowWalletsCommand::execute()
{
    auto wallets = _client.getWallets();

    if (wallets.empty()) {
        _client.output("no wallets saved");
    }
    else {
        for (const auto& wallet : wallets) {
            _client.output(wallet.first + " - " + wallet.second);
        }
    }
}


LastBlockInfoCommand::LastBlockInfoCommand(Client& client)
  : Command(client, 0)
{}


const std::string& LastBlockInfoCommand::name() const noexcept
{
    static std::string name{ "last_block_info" };
    return name;
}


const std::string& LastBlockInfoCommand::description() const noexcept
{
    static std::string description{ "Get last block info" };
    return description;
}


const std::string& LastBlockInfoCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool LastBlockInfoCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    return true;
}


void LastBlockInfoCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    LOG_INFO << "last_block_info";
    _client._web_socket_client.send(websocket::Command::CALL_LAST_BLOCK_INFO, base::json::Value::object());
}


AccountInfoCommand::AccountInfoCommand(Client& client)
  : Command(client, 1)
{}


const std::string& AccountInfoCommand::name() const noexcept
{
    static std::string name{ "account_info" };
    return name;
}


const std::string& AccountInfoCommand::description() const noexcept
{
    static std::string description{ "Get account info by specific address" };
    return description;
}


const std::string& AccountInfoCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<address of base58/contact name>" };
    return help_message;
}


bool AccountInfoCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    auto contacts = _client.getContacts();
    auto contact = contacts.find(arguments[0]);
    _address = contact != contacts.end() ? contact->second : takeAddress(_client, arguments[0]);
    if (!_address) {
        return false;
    }

    return true;
}


void AccountInfoCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    LOG_INFO << "account_info for address: " << _address.value();
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(_address.value());
    _client._web_socket_client.send(websocket::Command::CALL_ACCOUNT_INFO, std::move(request_args));
}


FeeInfoCommand::FeeInfoCommand(Client& client)
  : Command(client, 0)
{}


const std::string& FeeInfoCommand::name() const noexcept
{
    static std::string name{ "fee_info" };
    return name;
}


const std::string& FeeInfoCommand::description() const noexcept
{
    static std::string description{ "Asks the recommended commission for transactions" };
    return description;
}


const std::string& FeeInfoCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool FeeInfoCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }

    return true;
}


void FeeInfoCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }

    LOG_INFO << "Fee info";
    _client._web_socket_client.send(websocket::Command::CALL_FEE_INFO, base::json::Value::object());
}


SubscribeAccountInfoCommand::SubscribeAccountInfoCommand(Client& client)
  : Command(client, 1)
{}


const std::string& SubscribeAccountInfoCommand::name() const noexcept
{
    static std::string name{ "subscribe_account_info" };
    return name;
}


const std::string& SubscribeAccountInfoCommand::description() const noexcept
{
    static std::string description{ "Subscribe on updates account info by specific address" };
    return description;
}


const std::string& SubscribeAccountInfoCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<address of base58/contact name>" };
    return help_message;
}


bool SubscribeAccountInfoCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    auto contacts = _client.getContacts();
    auto contact = contacts.find(arguments[0]);
    _address = contact != contacts.end() ? contact->second : takeAddress(_client, arguments[0]);
    if (!_address) {
        return false;
    }

    return true;
}


void SubscribeAccountInfoCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    LOG_INFO << "account_info for address: " << _address.value();
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(_address.value());
    _client._web_socket_client.send(websocket::Command::SUBSCRIBE_ACCOUNT_INFO, std::move(request_args));
}


UnsubscribeAccountInfoCommand::UnsubscribeAccountInfoCommand(Client& client)
  : Command(client, 1)
{}


const std::string& UnsubscribeAccountInfoCommand::name() const noexcept
{
    static std::string name{ "unsubscribe_last_block_info" };
    return name;
}


const std::string& UnsubscribeAccountInfoCommand::description() const noexcept
{
    static std::string description{ "Unsubscribe from account info updates by specific address" };
    return description;
}


const std::string& UnsubscribeAccountInfoCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<address of base58/contact name>" };
    return help_message;
}


bool UnsubscribeAccountInfoCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    auto contacts = _client.getContacts();
    auto contact = contacts.find(arguments[0]);
    _address = contact != contacts.end() ? contact->second : takeAddress(_client, arguments[0]);
    if (!_address) {
        return false;
    }

    return true;
}


void UnsubscribeAccountInfoCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    LOG_INFO << "account_info for address: " << _address.value();
    auto request_args = base::json::Value::object();
    request_args["address"] = websocket::serializeAddress(_address.value());
    _client._web_socket_client.send(websocket::Command::UNSUBSCRIBE_ACCOUNT_INFO, std::move(request_args));
}


SubscribeLastBlockInfoCommand::SubscribeLastBlockInfoCommand(Client& client)
  : Command(client, 0)
{}


const std::string& SubscribeLastBlockInfoCommand::name() const noexcept
{
    static std::string name{ "subscribe_last_block_info" };
    return name;
}


const std::string& SubscribeLastBlockInfoCommand::description() const noexcept
{
    static std::string description{ "Get last block info when apeared new block" };
    return description;
}


const std::string& SubscribeLastBlockInfoCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool SubscribeLastBlockInfoCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    return true;
}


void SubscribeLastBlockInfoCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    LOG_INFO << "subscription last_block_info";
    _client._web_socket_client.send(websocket::Command::SUBSCRIBE_LAST_BLOCK_INFO, base::json::Value::object());
}


UnsubscribeLastBlockInfoCommand::UnsubscribeLastBlockInfoCommand(Client& client)
  : Command(client, 0)
{}


const std::string& UnsubscribeLastBlockInfoCommand::name() const noexcept
{
    static std::string name{ "unsubscribe_last_block_info" };
    return name;
}


const std::string& UnsubscribeLastBlockInfoCommand::description() const noexcept
{
    static std::string description{ "Unsubscribe from get last block info when apeared new block" };
    return description;
}


const std::string& UnsubscribeLastBlockInfoCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "" };
    return help_message;
}


bool UnsubscribeLastBlockInfoCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    return true;
}


void UnsubscribeLastBlockInfoCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    LOG_INFO << "unsubscription last_block_info";
    _client._web_socket_client.send(websocket::Command::UNSUBSCRIBE_LAST_BLOCK_INFO, base::json::Value::object());
}


FindTransactionCommand::FindTransactionCommand(Client& client)
  : Command(client, 1)
  , _hash_transaction(base::FixedBytes<base::Sha256::LENGTH>())
{}


const std::string& FindTransactionCommand::name() const noexcept
{
    static std::string name{ "find_transaction" };
    return name;
}


const std::string& FindTransactionCommand::description() const noexcept
{
    static std::string description{ "Get transaction by specific hash" };
    return description;
}


const std::string& FindTransactionCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<transaction hash at hex>" };
    return help_message;
}


bool FindTransactionCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    auto hash = takeHash(_client, arguments[0]);
    if (!hash) {
        return false;
    }
    _hash_transaction = hash.value();
    return true;
}


void FindTransactionCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    LOG_INFO << "find_transaction by hash: " << _hash_transaction;
    auto request_args = base::json::Value::object();
    request_args["hash"] = websocket::serializeHash(_hash_transaction);
    _client._web_socket_client.send(websocket::Command::CALL_FIND_TRANSACTION, std::move(request_args));
}


FindTransactionStatusCommand::FindTransactionStatusCommand(Client& client)
  : Command(client, 1)
  , _hash_transaction(base::FixedBytes<base::Sha256::LENGTH>())
{}


const std::string& FindTransactionStatusCommand::name() const noexcept
{
    static std::string name{ "find_transaction_status" };
    return name;
}


const std::string& FindTransactionStatusCommand::description() const noexcept
{
    static std::string description{ "Get transaction status by specific hash" };
    return description;
}


const std::string& FindTransactionStatusCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<transaction hash at hex>" };
    return help_message;
}


bool FindTransactionStatusCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    auto hash = takeHash(_client, arguments[0]);
    if (!hash) {
        return false;
    }
    _hash_transaction = hash.value();
    return true;
}


void FindTransactionStatusCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    LOG_INFO << "find_transaction_status by hash: " << _hash_transaction;
    auto request_args = base::json::Value::object();
    request_args["hash"] = websocket::serializeHash(_hash_transaction);
    _client._web_socket_client.send(websocket::Command::CALL_FIND_TRANSACTION_STATUS, std::move(request_args));
}


FindBlockCommand::FindBlockCommand(Client& client)
  : Command(client, 1)
  , _hash_block(base::FixedBytes<base::Sha256::LENGTH>())
{}


const std::string& FindBlockCommand::name() const noexcept
{
    static std::string name{ "find_block" };
    return name;
}


const std::string& FindBlockCommand::description() const noexcept
{
    static std::string description{ "Get block by specific hash" };
    return description;
}


const std::string& FindBlockCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<block hash at hex/depth of block>" };
    return help_message;
}


bool FindBlockCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _hash_block = takeHash(_client, arguments[0], true);
    if (!_hash_block) {
        _depth = takeDepth(_client, arguments[0]);
        if (!_depth) {
            return false;
        }
    }
    return true;
}


void FindBlockCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }

    if (_hash_block) {
        LOG_INFO << "Request for a block with a hash " << _hash_block.value() << " is sent: ";
    }
    else {
        LOG_INFO << "Request for a block with a depth " << _depth.value() << " is sent: ";
    }
    auto request_args = base::json::Value::object();
    if (_hash_block) {
        request_args["hash"] = websocket::serializeHash(_hash_block.value());
    }
    else if (_depth) {
        request_args["depth"] = websocket::serializeDepth(_depth.value());
    }
    _client._web_socket_client.send(websocket::Command::CALL_FIND_BLOCK, std::move(request_args));
}


TransferCommand::TransferCommand(Client& client)
  : Command(client, 4)
{}


const std::string& TransferCommand::name() const noexcept
{
    static std::string name{ "transfer" };
    return name;
}


const std::string& TransferCommand::description() const noexcept
{
    static std::string description{ "Transfer coins to specific address" };
    return description;
}


const std::string& TransferCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{
        "<path to folder with key or name of the wallet> <address of recipient at base58/contact name of recipient> <fee for transaction> <amount of coins for transfer>"
    };
    return help_message;
}


bool TransferCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    auto wallets = _client.getWallets();

    _keys_path = wallets.find(arguments[0]) != wallets.end() ? wallets[arguments[0]] : arguments[0];

    auto contacts = _client.getContacts();
    auto contact = contacts.find(arguments[1]);
    _to_address = contact != contacts.end() ? contact->second : takeAddress(_client, arguments[1]);
    if (!_to_address) {
        return false;
    }

    auto fee = takeFee(_client, arguments[2]);
    if (!fee) {
        return false;
    }
    _fee = fee.value();

    auto amount = takeAmount(_client, arguments[3]);
    if (!amount) {
        return false;
    }
    _amount = amount.value();

    return true;
}


void TransferCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }

    auto private_key_path = base::config::makePrivateKeyPath(_keys_path);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(_to_address.value());
    txb.setAmount(_amount);
    txb.setTimestamp(base::Time::now());
    txb.setFee(_fee);
    txb.setData({});
    auto tx = std::move(txb).build();

    tx.sign(private_key);


    auto tx_hash = tx.hashOfTransaction();
    _client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Transfer from " << from_address << " to " << _to_address.value() << " with amount " << _amount;

    auto request_args = websocket::serializeTransaction(tx);
    _client._web_socket_client.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


ContractCallCommand::ContractCallCommand(Client& client)
  : Command(client, 5)
{}


const std::string& ContractCallCommand::name() const noexcept
{
    static std::string name{ "contract_call" };
    return name;
}


const std::string& ContractCallCommand::description() const noexcept
{
    static std::string description{ "Call deployed contract" };
    return description;
}


const std::string& ContractCallCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{
        "<path to folder with key or name of the wallet> <address of contract at base58/contact name of contract> <fee for contract call> <amount of coins for call> <message for call at hex>"
    };
    return help_message;
}


bool ContractCallCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    auto wallets = _client.getWallets();

    _keys_path = wallets.find(arguments[0]) != wallets.end() ? wallets[arguments[0]] : arguments[0];

    auto contacts = _client.getContacts();
    auto contact = contacts.find(arguments[1]);
    _to_address = contact != contacts.end() ? contact->second : takeAddress(_client, arguments[1]);
    if (!_to_address) {
        return false;
    }

    auto fee = takeFee(_client, arguments[2]);
    if (!fee) {
        return false;
    }
    _fee = fee.value();

    auto amount = takeAmount(_client, arguments[3]);
    if (!amount) {
        return false;
    }
    _amount = amount.value();

    auto message = takeMessage(_client, arguments[4]);
    if (!message) {
        return false;
    }
    _message = message.value();

    return true;
}


void ContractCallCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    auto wallets = _client.getWallets();

    auto private_key_path = base::config::makePrivateKeyPath(_keys_path);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(_to_address.value());
    txb.setAmount(_amount);
    txb.setTimestamp(base::Time::now());
    txb.setFee(_fee);
    txb.setData(_message);
    auto tx = std::move(txb).build();

    tx.sign(private_key);

    auto tx_hash = tx.hashOfTransaction();
    _client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Contract_call from " << from_address << ", to " << _to_address.value() << ", amount " << _amount
             << ",fee " << _fee << ", message " << _message;

    auto request_args = websocket::serializeTransaction(tx);
    _client._web_socket_client.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


PushContractCommand::PushContractCommand(Client& client)
  : Command(client, 5)
{}


const std::string& PushContractCommand::name() const noexcept
{
    static std::string name{ "push_contract" };
    return name;
}


const std::string& PushContractCommand::description() const noexcept
{
    static std::string description{ "Deploy compiled contract" };
    return description;
}


const std::string& PushContractCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{
        "<path to folder with key or name of the wallet> <fee for contract call> <amount of coins for call><path to compiled contract data files> <message for initializing contract at hex>"
    };
    return help_message;
}


bool PushContractCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    auto wallets = _client.getWallets();

    _keys_path = wallets.find(arguments[0]) != wallets.end() ? wallets[arguments[0]] : arguments[0];

    auto fee = takeFee(_client, arguments[1]);
    if (!fee) {
        return false;
    }
    _fee = fee.value();

    auto amount = takeAmount(_client, arguments[2]);
    if (!amount) {
        return false;
    }
    _amount = amount.value();

    _contract_path = arguments[3];

    auto message = takeMessage(_client, arguments[4]);
    if (!message) {
        return false;
    }
    _message = message.value();

    return true;
}


void PushContractCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }


    auto private_key_path = base::config::makePrivateKeyPath(_keys_path);
    auto private_key = base::Secp256PrivateKey::load(private_key_path);
    auto from_address = lk::Address(private_key.toPublicKey());

    lk::TransactionBuilder txb;
    txb.setFrom(from_address);
    txb.setTo(lk::Address::null());
    txb.setAmount(_amount);
    txb.setTimestamp(base::Time::now());
    txb.setFee(_fee);

    if (_message.isEmpty()) {
        auto code_binary_file_path = _contract_path / std::filesystem::path(config::CONTRACT_BINARY_FILE);
        if (!std::filesystem::exists(code_binary_file_path)) {
            _client.output("Error the file with this path[" + code_binary_file_path.string() + "] does not exist");
            return;
        }
        std::ifstream file(code_binary_file_path, std::ios::binary);
        auto buf = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        _message = base::fromHex<base::Bytes>(buf);
    }
    txb.setData(_message);

    auto tx = std::move(txb).build();

    tx.sign(private_key);

    auto tx_hash = tx.hashOfTransaction();
    _client.output("Transaction with hash[hex]: " + tx_hash.toHex());

    LOG_INFO << "Push_contract from " << from_address << ", amount " << _amount << ", fee " << _fee << ", message "
             << _message;

    auto request_args = websocket::serializeTransaction(tx);
    _client._web_socket_client.send(websocket::Command::SUBSCRIBE_PUSH_TRANSACTION, std::move(request_args));
}


LoginCommand::LoginCommand(Client& client)
  : Command(client, 1)
{}


const std::string& LoginCommand::name() const noexcept
{
    static std::string name{ "login" };
    return name;
}


const std::string& LoginCommand::description() const noexcept
{
    static std::string description{ "try login to Clover node" };
    return description;
}


const std::string& LoginCommand::argumentsHelpMessage() const noexcept
{
    static std::string help_message{ "<login to the connected node>" };
    return help_message;
}


bool LoginCommand::prepareArgs()
{
    auto arguments = parseAllArguments(_args);
    if (arguments.size() != _count_arguments) {
        _client.output("Wrong number of arguments for the " + name() + " command");
        return false;
    }
    _login = arguments[0];

    return true;
}


void LoginCommand::execute()
{
    if (!_client.isConnected()) {
        _client.output("You have to connect to likelib node");
        return;
    }

    auto request_args = websocket::serializeLogin(_login);
    LOG_INFO << "try login to node"; // TODO change
    _client._web_socket_client.send(websocket::Command::LOGIN, std::move(request_args));
}
