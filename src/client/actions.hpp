#pragma once

#include "base/config.hpp"
#include "base/subprogram_router.hpp"
#include "bc/address.hpp"
#include "base/log.hpp"
#include "bc/types.hpp"
#include "rpc/rpc.hpp"

#include <iostream>
#include <string_view>


class ActionBase
{
  public:
    //====================================
    explicit ActionBase(base::SubprogramRouter& router);
    virtual ~ActionBase() = default;
    //====================================
    virtual const std::string_view& getName() const = 0;
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
    //====================================
    explicit ActionTransfer(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    bc::Address _to_address{bc::Address::null()};
    bc::Balance _amount;
    bc::Balance _fee;
    std::filesystem::path _keys_dir;
    //====================================
};


class ActionGetBalance : public ActionBase
{
  public:
    //====================================
    explicit ActionGetBalance(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    bc::Address _account_address{bc::Address::null()};
    //====================================
};


class ActionTestConnection : public ActionBase
{
  public:
    //====================================
    explicit ActionTestConnection(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    bc::Address _account_address{bc::Address::null()};
    //====================================
};


class ActionCreateContract : public ActionBase
{
  public:
    //====================================
    explicit ActionCreateContract(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    std::filesystem::path _keys_dir;
    bc::Balance _amount;
    bc::Balance _gas;
    std::string _compiled_contract;
    std::string _message;
    //====================================
};


class ActionMessageCall : public ActionBase
{
  public:
    //====================================
    explicit ActionMessageCall(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    bc::Address _to_address{bc::Address::null()};
    bc::Balance _amount;
    bc::Balance _gas;
    std::filesystem::path _keys_dir;
    std::string _message;
    //====================================
};


class ActionCompile : public ActionBase
{
  public:
    //====================================
    explicit ActionCompile(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::filesystem::path _code_file_path;
    //====================================
};


class ActionGenerateKeys : public ActionBase
{
  public:
    //====================================
    explicit ActionGenerateKeys(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::filesystem::path _keys_dir;
    //====================================
};