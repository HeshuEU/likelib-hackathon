#include "client.hpp"
#include "actions.hpp"

#include <readline/history.h>
#include <readline/readline.h>

#include <iostream>

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

std::string parseActionName(std::string& input)
{
    clearSpaces(input);
    std::size_t end{ 0 };
    while (end < input.size() && input[end] != ' ') {
        end++;
    }
    auto action_name = input.substr(0, end);
    input.erase(0, end);
    clearSpaces(input);
    return action_name;
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


std::vector<Client::Command> Client::_commands{};


std::vector<Client::Command> Client::initCommands()
{
    std::vector<Client::Command> commands;

    commands.push_back({ "help", "Show help message", "", 0, std::bind(&Client::help, this, std::placeholders::_2) });

    commands.push_back(
      { "exit", "Exit fron likelib client", "", 0, std::bind(&Client::exit, this, std::placeholders::_2) });

    commands.push_back({ "connect",
                         "Connect client to specific likelib node",
                         "<ip and port of likelib node>",
                         1,
                         std::bind(&Client::connect, this, std::placeholders::_2) });
    commands.push_back({ "disconnect",
                         "Disconnect client from likelib node",
                         "",
                         0,
                         std::bind(&Client::disconnect, this, std::placeholders::_2) });
    commands.push_back({ "compile",
                         "Compile solidity code to binary format",
                         "<path to solidity code file>",
                         1,
                         &compile_solidity_code });
    commands.push_back({ "encode",
                         "Encode message for contract call",
                         "<path to folder with compiled contract data files> <message for encode>",
                         2,
                         &encode_message });
    commands.push_back({ "decode",
                         "Decode message which was returned by contract call",
                         "<path to folder with compiled contract data files> <message for decode>",
                         2,
                         &decode_message });
    commands.push_back({ "keys_generate",
                         "Generate new key and store to specific folder",
                         "<path to folder to save key>",
                         1,
                         &generate_keys });
    commands.push_back({ "keys_info", "Print info about specified key", "<path to folder with key>", 1, &keys_info });
    commands.push_back({ "add_wallet",
                         "Remembers the path to the key, under the selected name",
                         "<name of new wallet> <path to foldef with key>",
                         2,
                         &add_wallet });
    commands.push_back(
      { "delete_wallet", "Deletes the wallet with the selected name", "<wallet name to delete>", 1, &delete_wallet });
    commands.push_back(
      { "show_wallets", "Outputs all memorized wallet names and their corresponding paths", "", 0, &show_wallets });
    commands.push_back({ "last_block_info", "Get last block info", "", 0, &call_last_block_info });
    commands.push_back(
      { "account_info", "Get account info by specific address", "<address of base58>", 1, &call_account_info });
    commands.push_back({ "subscribe_account_info",
                         "Subscribe on updates account info by specific address",
                         "<address of base58>",
                         1,
                         &subscribe_account_info });
    commands.push_back({ "unsubscribe_account_info",
                         "Unsubscribe from account info updates by specific address",
                         "<address at base58>",
                         1,
                         &unsubscribe_account_info });
    commands.push_back(
      { "subscribe_last_block_info", "Get last block info when apeared new block", "", 0, &subscribe_last_block_info });
    commands.push_back({ "unsubscribe_last_block_info",
                         "Get last block info when apeared new block",
                         "",
                         0,
                         &unsubscribe_last_block_info });
    commands.push_back({ "find_transaction",
                         "Get transaction by specific hash",
                         "<transaction hash at hex>",
                         1,
                         &call_find_transaction });
    commands.push_back({ "find_transaction_status",
                         "Get transaction status by specific hash",
                         "<transaction hash at hex>",
                         1,
                         &call_find_transaction_status });
    commands.push_back({ "find_block", "Get block by specific hash", "<block hash at hex>", 1, &call_find_block });
    commands.push_back({ "transfer",
                         "Transfer coins to specific address",
                         "<path to folder with key\\or name of the wallet> <address of recipient at base58>"
                         " <fee for transaction> <amount of coins for transfer>",
                         4,
                         &transfer });
    commands.push_back({ "contract_call",
                         "Call deployed contract",
                         "<path to folder with key\\or name of the wallet> <address of contract at base58> "
                         "<fee for contract call> <amount of coins for call> <message for call at hex>",
                         5,
                         &contract_call });
    commands.push_back({ "push_contract",
                         "Deploy compiled contract",
                         "<path to folder with key\\or name of the wallet> <fee for contract call> <amount of coins for call>"
                         "<path to compiled contract data files> <message for initializing contract at hex>",
                         6,
                         &push_contract });
    return commands;
}


void Client::chooseAction(std::string& input)
{
    auto action_name = parseActionName(input);
    if (action_name.empty()) {
        return;
    }
    const auto& arguments = parseAllArguments(input);
    for (const auto& command : _commands) {
        if (action_name == command._command_name) {
            if (arguments.size() != command._count_arguments) {
                output("Wrong number of arguments for the " + command._command_name + " command");
                return;
            }
            try {
                command._function(*this, arguments);
            }
            catch (const base::Error& e) {
                output(e.what());
                LOG_ERROR << e.what();
            }
            return;
        }
    }
    output("There is no command with the name " + action_name);
}


void Client::help(const std::vector<std::string>& arguments)
{
    for (const auto& command : _commands) {
        output("- " + command._command_name + " " + command._command_arguments_msg + "\n\t\t" +
               command._command_description + "\n");
    }
}


void Client::exit(const std::vector<std::string>& arguments)
{
    _io_context.stop();
    _web_socket_client.disconnect();
    _exit = true;
}


void Client::connect(const std::vector<std::string>& arguments)
{
    auto host = arguments[0];
    _connected = _web_socket_client.connect(host);
    if (_connected) {
        _host = host;
        _thread = std::thread{ [&]() {
            if (_io_context.stopped()) {
                _io_context.restart();
            }
            _io_context.run();
        } };
        output("Client connected to host " + host);
    }
    else {
        output("Can't connect to host " + host);
    }
}


void Client::disconnect(const std::vector<std::string>& arguments)
{
    _connected = false;
    _io_context.stop();
    _web_socket_client.disconnect();
    _thread.join();
}


void Client::processLine(std::string line)
{
    clearSpaces(line);
    if (line.empty()) {
        return;
    }
    else {
        add_history(line.c_str());
        chooseAction(line);
    }
}


char** Client::characterNameCompletion(const char* text, int start, int end)
{
    rl_attempted_completion_over = 1;
    if (start == 0) {
        return rl_completion_matches(text, characterNameGenerator);
    }
    return rl_completion_matches(text, rl_filename_completion_function);
}

char* Client::characterNameGenerator(const char* text, int state)
{
    static std::size_t list_index, len;
    const char* name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while (list_index < _commands.size()) {
        name = _commands[list_index++]._command_name.c_str();
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    return nullptr;
}


Client::Client()
  : _prompt("> ")
  , _io_context{}
  , _web_socket_client{ _io_context,
                        std::bind(&Client::printReceivedData, this, std::placeholders::_1, std::placeholders::_2),
                        [this]() {
                            output("Disconnected from likelib node\n");
                            _connected = false;
                        } }

{
    rl_attempted_completion_function = characterNameCompletion;
    _commands = initCommands();
}


void Client::run()
{
    while (!_exit) {
        const auto line = readline(_prompt.c_str());

        if (line && *line) {
            processLine(line);
        }
        rl_free(line);
    }
}


void Client::output(const std::string& str)
{
    _out_mutex.lock();
    std::cout << str << std::endl;
    _out_mutex.unlock();
}


void Client::remoteOutput(const std::string& str)
{
    _out_mutex.lock();
    deactivateReadline();
    std::cin.clear();
    std::cout << str << std::endl;
    reactivateReadline();
    _out_mutex.unlock();
}


const std::map<std::string, std::string>& Client::getWallets() const
{
    return _wallets;
}


void Client::addWallet(const std::string wallet_name, const std::filesystem::path& keys_dir)
{
    _wallets[wallet_name] = keys_dir;
}


void Client::deleteWallet(const std::string wallet_name)
{
    _wallets.erase(wallet_name);
}


bool Client::isConnected() const
{
    return _connected;
}


websocket::WebSocketClient& Client::getWebSocket()
{
    return _web_socket_client;
}


void Client::deactivateReadline()
{
    _saved_point = rl_point;
    _saved_line = std::string(rl_line_buffer, rl_end);

    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_redisplay();
}


void Client::reactivateReadline()
{
    rl_set_prompt(_prompt.c_str());
    rl_point = _saved_point;
    rl_replace_line(_saved_line.c_str(), 0);
    rl_redisplay();
}


void Client::printReceivedData(websocket::Command::Id command_id, base::json::Value received_message)
{
    remoteOutput("Received data: " + received_message.serialize() + "\n");
    // TODO
}