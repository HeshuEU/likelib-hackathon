#include "client.hpp"

#include <readline/history.h>
#include <readline/readline.h>

#include <iostream>

std::string parseCommandName(std::string& input)
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


bool checkCommandNameFull(std::string input)
{
    clearSpaces(input);
    std::size_t end{ 0 };
    while (end < input.size() && input[end] != ' ') {
        end++;
    }
    auto action_name = input.substr(0, end);
    input.erase(0, end);
    if (!input.empty()) {
        return true;
    }
    return false;
}


std::vector<Command*> Client::_commands{};


std::vector<Command*> Client::initCommands()
{
    std::vector<Command*> commands;

    commands.emplace_back(new HelpCommand{ *this });
    commands.emplace_back(new ConnectCommand{ *this });
    commands.emplace_back(new DisconnectCommand{ *this });
    commands.emplace_back(new ExitCommand{ *this });
    commands.emplace_back(new CompilyCommand{ *this });
    commands.emplace_back(new EncodeCommand{ *this });
    commands.emplace_back(new DecodeCommand{ *this });
    commands.emplace_back(new KeysGenerateCommand{ *this });
    commands.emplace_back(new KeysInfoCommand{ *this });
    commands.emplace_back(new AddContactCommand{ *this });
    commands.emplace_back(new DeleteContactCommand{ *this });
    commands.emplace_back(new ShowContactsCommand{ *this });
    commands.emplace_back(new AddWalletCommand{ *this });
    commands.emplace_back(new DeleteWalletCommand{ *this });
    commands.emplace_back(new ShowWalletsCommand{ *this });
    commands.emplace_back(new LastBlockInfoCommand{ *this });
    commands.emplace_back(new AccountInfoCommand{ *this });
    commands.emplace_back(new FeeInfoCommand{ *this });
    commands.emplace_back(new SubscribeAccountInfoCommand{ *this });
    commands.emplace_back(new UnsubscribeAccountInfoCommand{ *this });
    commands.emplace_back(new SubscribeLastBlockInfoCommand{ *this });
    commands.emplace_back(new UnsubscribeLastBlockInfoCommand{ *this });
    commands.emplace_back(new FindTransactionCommand{ *this });
    commands.emplace_back(new FindTransactionStatusCommand{ *this });
    commands.emplace_back(new FindBlockCommand{ *this });
    commands.emplace_back(new TransferCommand{ *this });
    commands.emplace_back(new ContractCallCommand{ *this });
    commands.emplace_back(new PushContractCommand{ *this });
    //commands.emplace_back(new LoginCommand{ *this });

    return commands;
}


std::optional<Command*> Client::chooseCommand(const std::string& command_name)
{
    for (const auto& command : _commands) {
        if (command_name == command->name()) {
            return command;
        }
    }
    return {};
}


void Client::processInput(std::string& input)
{
    auto command_name = parseCommandName(input);
    if (command_name.empty()) {
        return;
    }

    auto command = chooseCommand(command_name);
    if (command) {
        try {
            command.value()->run(input);
        }
        catch (const base::Error& e) {
            output(e.what());
            LOG_ERROR << e.what();
        }
        return;
    }
    output("There is no command with the name " + command_name);
}


void Client::processLine(std::string line)
{
    clearSpaces(line);
    if (line.empty()) {
        return;
    }
    else {
        add_history(line.c_str());
        processInput(line);
    }
}


char** Client::characterNameCompletion(const char* text, int start, int end)
{
    rl_attempted_completion_over = 1;

    std::string input{ rl_line_buffer, static_cast<std::size_t>(rl_point) };
    if (checkCommandNameFull(input)) {
        return rl_completion_matches(text, completionGenerator);
    }
    return rl_completion_matches(text, commandGenerator);
    // return rl_completion_matches(text, rl_filename_completion_function);
}

char* Client::completionGenerator(const char* text, int state)
{
    std::string input{ rl_line_buffer, static_cast<std::size_t>(rl_point) };
    if (checkCommandNameFull(input)) {
        auto command_name = parseCommandName(input);
        auto command = chooseCommand(command_name);
        if (command) {
            auto completion = command.value()->completionGenerator(input);
            if (completion) {
                return strdup(completion.value().c_str());
            }
        }
        return rl_filename_completion_function(text, state);
    }
}


char* Client::commandGenerator(const char* text, int state)
{
    static std::size_t list_index, len;
    const char* name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while (list_index < _commands.size()) {
        name = _commands[list_index++]->name().c_str();
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


const std::map<std::string, std::optional<lk::Address>>& Client::getContacts() const
{
    return _contacts;
}


void Client::addWallet(const std::string wallet_name, const std::filesystem::path& keys_dir)
{
    _wallets[wallet_name] = keys_dir;
}


void Client::deleteWallet(const std::string wallet_name)
{
    _wallets.erase(wallet_name);
}


void Client::addContact(const std::string contact_name, const std::optional<lk::Address>& address)
{
    _contacts[contact_name] = address;
}


void Client::deleteContact(const std::string contact_name)
{
    _contacts.erase(contact_name);
}


bool Client::isConnected() const
{
    return _connected;
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