#pragma once

#include "core/types.hpp"

#include "websocket/client.hpp"
#include "websocket/tools.hpp"

#include <boost/thread.hpp>

#include "actions.hpp"

class Command;
class HelpCommand;
class ConnectCommand;
class DisconnectCommand;
class ExitCommand;
class CompilyCommand;
class EncodeCommand;
class DecodeCommand;
class KeysGenerateCommand;
class KeysInfoCommand;
class AddContactCommand;
class DeleteContactCommand;
class ShowContactsCommand;
class AddWalletCommand;
class DeleteWalletCommand;
class ShowWalletsCommand;
class LastBlockInfoCommand;
class AccountInfoCommand;
class FeeInfoCommand;
class SubscribeAccountInfoCommand;
class UnsubscribeAccountInfoCommand;
class SubscribeLastBlockInfoCommand;
class UnsubscribeLastBlockInfoCommand;
class FindTransactionCommand;
class FindTransactionStatusCommand;
class FindBlockCommand;
class TransferCommand;
class ContractCallCommand;
class PushContractCommand;
class LoginCommand;

class Client
{
    friend HelpCommand;
    friend ConnectCommand;
    friend DisconnectCommand;
    friend ExitCommand;
    friend CompilyCommand;
    friend EncodeCommand;
    friend DecodeCommand;
    friend KeysGenerateCommand;
    friend KeysInfoCommand;
    friend AddContactCommand;
    friend DeleteContactCommand;
    friend ShowContactsCommand;
    friend AddWalletCommand;
    friend DeleteWalletCommand;
    friend ShowWalletsCommand;
    friend LastBlockInfoCommand;
    friend AccountInfoCommand;
    friend FeeInfoCommand;
    friend SubscribeAccountInfoCommand;
    friend UnsubscribeAccountInfoCommand;
    friend SubscribeLastBlockInfoCommand;
    friend UnsubscribeLastBlockInfoCommand;
    friend FindTransactionCommand;
    friend FindTransactionStatusCommand;
    friend FindBlockCommand;
    friend TransferCommand;
    friend ContractCallCommand;
    friend PushContractCommand;
    friend LoginCommand;


  public:
    explicit Client();

    void run();

    void output(const std::string& str);
    void remoteOutput(const std::string& str);

    bool isConnected() const;

  private:
    std::string _prompt;
    std::string _saved_line;
    std::uint32_t _saved_point;

    std::mutex _out_mutex;

    bool _exit{ false };

    boost::asio::io_context _io_context;
    websocket::WebSocketClient _web_socket_client;

    bool _connected{ false };
    std::string _host;

    std::thread _thread;

    std::map<std::string, std::string> _wallets;
    std::map<std::string, std::optional<lk::Address>> _contacts;
    static std::vector<Command*> _commands;

    const std::map<std::string, std::string>& getWallets() const;
    const std::map<std::string, std::optional<lk::Address>>& getContacts() const;

    void addWallet(const std::string wallet_name, const std::filesystem::path& keys_dir);
    void deleteWallet(const std::string wallet_name);

    void addContact(const std::string contact_name, const std::optional<lk::Address>& address);
    void deleteContact(const std::string contact_name);

    std::vector<Command*> initCommands();

    void processLine(std::string line);

    void reactivateReadline();
    void deactivateReadline();

    static std::optional<Command*> chooseCommand(const std::string& command_name);
    void processInput(std::string& input);

    void printReceivedData(websocket::Command::Id command_id, base::json::Value received_message);

    static char** characterNameCompletion(const char* text, int start, int end);
    static char* commandGenerator(const char* text, int state);
    static char* completionGenerator(const char* text, int state);
};