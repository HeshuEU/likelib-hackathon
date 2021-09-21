#pragma once

#include "websocket/client.hpp"
#include "websocket/tools.hpp"

#include "core/address.hpp"
#include "core/types.hpp"

#include "client.hpp"

#include <string_view>

void clearSpaces(std::string& str);

std::string parseArgument(std::string& input);

std::vector<std::string> parseAllArguments(std::string& input);

class Client;

class Command
{
  public:
    Command(Client& client, std::size_t count_arguments = 0);
    virtual ~Command() = default;
    void run(const std::string& arguments);

    virtual const std::string& name() const noexcept = 0;
    virtual const std::string& description() const noexcept = 0;
    virtual const std::string& argumentsHelpMessage() const noexcept = 0;
    
    std::optional<std::string> completionGenerator(const std::string& input);
    std::size_t getCountArguments();

  protected:
    std::string _args;
    Client& _client;
    std::size_t _count_arguments;


    virtual bool prepareArgs() = 0;
    virtual void execute() = 0;
};


class HelpCommand final : public Command
{
  public:
    HelpCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
};


class ConnectCommand final : public Command
{
  public:
    ConnectCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _host;
};


class DisconnectCommand final : public Command
{
  public:
    DisconnectCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
};


class ExitCommand final : public Command
{
  public:
    ExitCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
};


class CompilyCommand final : public Command
{
  public:
    CompilyCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _code_path;
};


class EncodeCommand final : public Command
{
  public:
    EncodeCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _compiled_contract_folder_path;
    std::string _message;
};


class DecodeCommand final : public Command
{
  public:
    DecodeCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _compiled_contract_folder_path;
    std::string _message;
};


class KeysGenerateCommand final : public Command
{
  public:
    KeysGenerateCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::filesystem::path _keys_path;
};


class KeysInfoCommand final : public Command
{
  public:
    KeysInfoCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::filesystem::path _keys_path;
};


class AddContactCommand final : public Command
{
  public:
    AddContactCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _contact_name;
    std::optional<lk::Address> _address;
};


class DeleteContactCommand final : public Command
{
  public:
    DeleteContactCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _contact_name;
};


class ShowContactsCommand final : public Command
{
  public:
    ShowContactsCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _contact_name;
};


class AddWalletCommand final : public Command
{
  public:
    AddWalletCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _wallet_name;
    std::filesystem::path _keys_path;
};


class DeleteWalletCommand final : public Command
{
  public:
    DeleteWalletCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::string _wallet_name;
};


class ShowWalletsCommand final : public Command
{
  public:
    ShowWalletsCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
};


class LastBlockInfoCommand final : public Command
{
  public:
    LastBlockInfoCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
};


class AccountInfoCommand final : public Command
{
  public:
    AccountInfoCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::optional<lk::Address> _address;
};


class FeeInfoCommand final : public Command
{
  public:
    FeeInfoCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
};


class SubscribeAccountInfoCommand final : public Command
{
  public:
    SubscribeAccountInfoCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::optional<lk::Address> _address;
};

class UnsubscribeAccountInfoCommand final : public Command
{
  public:
    UnsubscribeAccountInfoCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::optional<lk::Address> _address;
};

class SubscribeLastBlockInfoCommand final : public Command
{
  public:
    SubscribeLastBlockInfoCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
};


class UnsubscribeLastBlockInfoCommand final : public Command
{
  public:
    UnsubscribeLastBlockInfoCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
};


class FindTransactionCommand final : public Command
{
  public:
    FindTransactionCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    base::Sha256 _hash_transaction;
};


class FindTransactionStatusCommand final : public Command
{
  public:
    FindTransactionStatusCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    base::Sha256 _hash_transaction;
};


class FindBlockCommand final : public Command
{
  public:
    FindBlockCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::optional<base::Sha256> _hash_block;
    std::optional<lk::BlockDepth> _depth;
};


class TransferCommand final : public Command
{
  public:
    TransferCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;
    std::optional<lk::Address> _to_address;
    lk::Balance _amount;
    lk::Fee _fee;
    std::filesystem::path _keys_path;
};


class ContractCallCommand final : public Command
{
  public:
    ContractCallCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;

    std::optional<lk::Address> _to_address;
    lk::Balance _amount;
    lk::Fee _fee;
    std::filesystem::path _keys_path;
    base::Bytes _message;
};


class PushContractCommand final : public Command
{
  public:
    PushContractCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;

    std::filesystem::path _keys_path;
    lk::Balance _amount;
    lk::Fee _fee;
    std::filesystem::path _contract_path;
    base::Bytes _message;
};


class LoginCommand final : public Command
{
  public:
    LoginCommand(Client& client);
    const std::string& name() const noexcept override;
    const std::string& description() const noexcept override;
    const std::string& argumentsHelpMessage() const noexcept override;

  protected:
    bool prepareArgs() override;
    void execute() override;

    std::string _login;
};
