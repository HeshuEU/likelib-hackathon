#pragma once

#include "base/config.hpp"
#include "base/subprogram_router.hpp"
#include "bc/address.hpp"
#include "base/log.hpp"
#include "bc/types.hpp"
#include "rpc/rpc.hpp"

#include <iostream>


int testConnection(base::SubprogramRouter& router);
int getBalance(base::SubprogramRouter& router);
int transfer(base::SubprogramRouter& router);
int createContract(base::SubprogramRouter& router);
int messageCall(base::SubprogramRouter& router);
int compileCode(base::SubprogramRouter& router);
int generateKeys(base::SubprogramRouter& router);


class ActionBase
{
public:
    //====================================
    explicit ActionBase(base::SubprogramRouter& router);
    virtual ~ActionBase() = default;
    //====================================
    virtual void setupOptionsParser(base::ProgramOptionsParser& parser) = 0;
    virtual int loadOptions(const base::ProgramOptionsParser& parser) = 0;
    virtual int execute() = 0;
    //====================================
    int run();
    //====================================
protected:
    base::SubprogramRouter& _router;
};


class ActionTransfer : public ActionBase
{
public:
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
private:
    //====================================
    static constexpr const char* CONFIG_OPTION = "config";
    static constexpr const char* HOST_OPTION = "host";
    static constexpr const char* TO_ADDRESS_OPTION = "to";
    static constexpr const char* AMOUNT_OPTION = "amount";
    static constexpr const char* KEYS_DIRECTORY_OPTION = "keys";
    static constexpr const char* FEE_OPTION = "fee";
    //====================================
    std::string _host_address;
    bc::Address _from_address{bc::Address::null()};
    bc::Address _to_address{bc::Address::null()};
    bc::Balance _amount;
    bc::Balance _fee;
    std::filesystem::path _keys_path;
    std::optional<base::RsaPublicKey> _public_key;
    std::optional<base::RsaPrivateKey> _private_key;
    //====================================
 };


class ActionGetBalance : public ActionBase
{
public:
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
private:
    //====================================
    static constexpr const char* CONFIG_OPTION = "config";
    static constexpr const char* HOST_OPTION = "host";
    static constexpr const char* ADDRESS_OPTION = "address";
    //====================================
    std::string _host_address;
    bc::Address _account_address{bc::Address::null()};
    //====================================
};


class ActionTestConnection : public ActionBase
{
public:
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
private:
    //====================================
    static constexpr const char* HOST_OPTION = "host";
    static constexpr const char* CODE_PATH_OPTION = "code";
    static constexpr const char* AMOUNT_OPTION = "amount";
    static constexpr const char* GAS_OPTION = "gas";
    static constexpr const char* INITIAL_MESSAGE_OPTION = "init";
    //====================================
    std::string _host_address;
    bc::Address _account_address{bc::Address::null()};
    //====================================
};


class ActionCreateContract : public ActionBase
{
public:
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
private:
    //====================================
    static constexpr const char* HOST_OPTION = "host";
    //====================================
    std::string _host_address;
    bc::Address _account_address{bc::Address::null()};
    //====================================
};