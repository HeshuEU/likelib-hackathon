#pragma once

#include "client/subprogram_router.hpp"

#include "core/address.hpp"
#include "core/types.hpp"

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
    bool _is_http_mode{ false };
    //====================================
};


class ActionNodeInfo : public ActionBase
{
  public:
    //====================================
    explicit ActionNodeInfo(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    bool _is_http_mode{ false };
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


class ActionKeysInfo : public ActionBase
{
  public:
    //====================================
    explicit ActionKeysInfo(base::SubprogramRouter& router);
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
    lk::Address _account_address{ lk::Address::null() };
    bool _is_http_mode{ false };
    //====================================
};


class ActionGetAccountInfo : public ActionBase
{
  public:
    //====================================
    explicit ActionGetAccountInfo(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    lk::Address _account_address{ lk::Address::null() };
    bool _is_http_mode{ false };
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


class ActionEncode : public ActionBase
{
  public:
    //====================================
    explicit ActionEncode(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::filesystem::path _compiled_code_folder_path;
    std::string _call_data;
    //====================================
};


class ActionDecode : public ActionBase
{
  public:
    //====================================
    explicit ActionDecode(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::filesystem::path _compiled_code_folder_path;
    std::string _data_to_decode;
    //====================================
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
    lk::Address _to_address{ lk::Address::null() };
    lk::Balance _amount;
    std::uint64_t _fee;
    std::filesystem::path _keys_dir;
    bool _is_http_mode{ false };
    //====================================
};


class ActionPushContract : public ActionBase
{
  public:
    //====================================
    explicit ActionPushContract(base::SubprogramRouter& router);
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
    lk::Balance _amount;
    std::uint64_t _fee;
    base::Bytes _message;
    bool _is_http_mode{ false };
    //====================================
};


class ActionContractCall : public ActionBase
{
  public:
    //====================================
    explicit ActionContractCall(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    lk::Address _to_address{ lk::Address::null() };
    lk::Balance _amount;
    std::uint64_t _fee;
    std::filesystem::path _keys_dir;
    std::string _message;
    bool _is_http_mode{ false };
    //====================================
};


class ActionGetTransaction : public ActionBase
{
  public:
    //====================================
    explicit ActionGetTransaction(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    base::Sha256 _transaction_hash{ base::Sha256::null() };
    bool _is_http_mode{ false };
    //====================================
};


class ActionGetTransactionStatus : public ActionBase
{
  public:
    //====================================
    explicit ActionGetTransactionStatus(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    base::Sha256 _transaction_hash{ base::Sha256::null() };
    bool _is_http_mode{ false };
    //====================================
};


class ActionGetBlock : public ActionBase
{
  public:
    //====================================
    explicit ActionGetBlock(base::SubprogramRouter& router);
    //====================================
    const std::string_view& getName() const override;
    void setupOptionsParser(base::ProgramOptionsParser& parser) override;
    int loadOptions(const base::ProgramOptionsParser& parser) override;
    int execute() override;
    //====================================
  private:
    //====================================
    std::string _host_address;
    base::Sha256 _block_hash{ base::Sha256::null() };
    std::uint64_t _block_number;
    bool _is_http_mode{ false };
    //====================================
};
