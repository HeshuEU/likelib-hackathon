#include "client.hpp"
#include "actions.hpp"

#include <readline/history.h>
#include <readline/readline.h>

#include <cli/clilocalsession.h>
#include <cli/remotecli.h>

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

void Client::chooseAction(std::string& input)
{
    auto action_name = parseActionName(input);
    if (action_name.empty()) {
        return;
    }
    const auto& arguments = parseAllArguments(input);
    try {
        if (action_name == "help") {
            if (arguments.size() != 0) {
                output("Wrong number of arguments for the help command");
                return;
            }
            help(*this);
        }
        else if (action_name == "connect") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the connect command");
                return;
            }
            if (_connected) {
                output("You are already connected to host " + _host + "\n disconnect before new reconnecting");
                return;
            }

            auto host = arguments[0];
            _connected = _web_socket_client.connect(host);
            if (_connected) {
                _host = host;
                thread = std::thread{[&](){
                    _io_context.run();
                }};
                output("Client connected to host " + host);
            }
            else {
                output("Can't connect to host " + host);
            }
        }
        else if (action_name == "disconnect") {
            if (arguments.size() != 0) {
                output("Wrong number of arguments for the disconnect command");
                return;
            }
            if (!_connected) {
                output("You aren't connected to the node");
                return;
            }

            _connected = false;
            _web_socket_client.disconnect();
            //_io_context.stop();
        }
        else if (action_name == "compile") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the compile command");
                return;
            }

            const auto path = arguments[0];
            compile_solidity_code(*this, path);
        }
        else if (action_name == "encode") {
            if (arguments.size() != 2) {
                output("Wrong number of arguments for the encode command");
                return;
            }

            const auto path = arguments[0];
            const auto message = arguments[1];
            encode_message(*this, path, message);
        }
        else if (action_name == "decode") {
            if (arguments.size() != 2) {
                output("Wrong number of arguments for the decode command");
                return;
            }

            const auto path = arguments[0];
            const auto message = arguments[1];
            decode_message(*this, path, message);
        }
        else if (action_name == "keys_generate") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the keys_generate command");
                return;
            }

            const auto path = arguments[0];
            generate_keys(*this, path);
        }
        else if (action_name == "keys_info") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the keys_info command");
                return;
            }

            const auto path = arguments[0];
            keys_info(*this, path);
        }
        else if (action_name == "last_block_info") {
            if (arguments.size() != 0) {
                output("Wrong number of arguments for the last_block_info command");
                return;
            }
            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            output(std::to_string(_web_socket_client.ready()));
            call_last_block_info(_web_socket_client);
        }
        else if (action_name == "account_info") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the account_info command");
                return;
            }
            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            const lk::Address address{ arguments[0] };
            call_account_info(_web_socket_client, address);
        }
        else if (action_name == "subscribe_account_info") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the subscribe_account_info command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            const lk::Address address{ arguments[0] };
            subscribe_account_info(_web_socket_client, address);
        }
        else if (action_name == "unsubscribe_account_info") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the unsubscribe_account_info command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            lk::Address address{ arguments[0] };
            unsubscribe_account_info(_web_socket_client, address);
        }
        else if (action_name == "find_transaction") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the find_transaction command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            const auto hash = base::Sha256::fromHex(arguments[0]);
            call_find_transaction(_web_socket_client, hash);
        }
        else if (action_name == "find_transaction_status") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the find_transaction_status command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            const auto hash = base::Sha256::fromHex(arguments[0]);
            call_find_transaction_status(_web_socket_client, hash);
        }
        else if (action_name == "find_block") {
            if (arguments.size() != 1) {
                output("Wrong number of arguments for the find_block command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            const auto hash = base::Sha256::fromHex(arguments[0]);
            call_find_block(_web_socket_client, hash);
        }
        else if (action_name == "transfer") {
            if (arguments.size() != 4) {
                output("Wrong number of arguments for the transfer command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            const std::filesystem::path& keys_dir{ arguments[0] };
            const lk::Address to_address{ arguments[1] };
            const lk::Fee fee{ std::stoull(arguments[2]) };
            const lk::Balance amount{ arguments[3] };         
            transfer(*this, _web_socket_client, to_address, amount, fee, keys_dir);
        }
        else if (action_name == "contract_call") {
            if (arguments.size() != 5) {
                output("Wrong number of arguments for the contract_call command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            const lk::Address to_address{ arguments[0] };
            const lk::Balance amount{ arguments[1] };
            const lk::Fee fee{ std::stoull(arguments[2]) };
            const std::filesystem::path& keys_dir{ arguments[3] };
            const auto message{ arguments[4] };
            contract_call(*this, _web_socket_client, to_address, amount, fee, keys_dir, message);
        }
        else if (action_name == "push_contract") {
            if (arguments.size() != 5) {
                output("Wrong number of arguments for the push_contract command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            const lk::Balance amount{ arguments[0] };
            const lk::Fee fee{ std::stoull(arguments[1]) };
            const std::filesystem::path& keys_dir{ arguments[2] };
            const std::filesystem::path& path_to_compiled_folder{ arguments[3] };
            const auto message{ arguments[4] };
            push_contract(*this, _web_socket_client, amount, fee, keys_dir, path_to_compiled_folder, message);
        }
        else if (action_name == "subscribe_last_block_info") {
            if (arguments.size() != 0) {
                output("Wrong number of arguments for the subscribe_last_block_info command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            subscribe_last_block_info(_web_socket_client);
        }
        else if (action_name == "unsubscribe_last_block_info") {
            if (arguments.size() != 0) {
                output("Wrong number of arguments for the unsubscribe_last_block_info command");
                return;
            }

            if (!_connected) {
                output("You have to connect to likelib node");
                return;
            }
            unsubscribe_last_block_info(_web_socket_client);
        }
        else {
            output("There is no command with the name " + action_name);
        }
    }
    catch (const base::Error& e) {
        output(e.what());
        LOG_ERROR << e.what();
    }
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

Client::Client()
  : _prompt("Likelib: ")
  , _io_context{}
  , _web_socket_client{ _io_context,
                        std::bind(&Client::printReceivedData, this, std::placeholders::_1, std::placeholders::_2),
                        [this]() {
                            output("Disconnected from likelib node\n");
                            _connected = false;
                        } }
{}


void Client::run()
{
    while (true) {
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