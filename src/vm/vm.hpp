#pragma once

#include "base/bytes.hpp"

#include "bc/types.hpp"

#include <evmc/evmc.hpp>

namespace vm
{

class SmartContractMessage;

class SmartContract
{
  public:
    SmartContract(const base::Bytes& contract_code);
    SmartContract(const SmartContract&) = default;
    SmartContract(SmartContract&&) = default;
    SmartContract& operator=(const SmartContract&) = default;
    SmartContract& operator=(SmartContract&&) = default;
    ~SmartContract() = default;

    SmartContractMessage createInitMessage(int64_t gas, base::Bytes& source, const base::Bytes& destination,
        const bc::Balance& value, const base::Bytes& input) const;

    SmartContractMessage createMessage(int64_t gas, base::Bytes& source, const base::Bytes& destination,
        const bc::Balance& value, const base::Bytes& input) const;

  private:
    evmc_revision _revision;
    base::Bytes _contract_code;
};


class SmartContractMessage
{
    friend SmartContract;

  public:
    SmartContractMessage(const SmartContractMessage&) = default;
    SmartContractMessage(SmartContractMessage&&) = default;
    SmartContractMessage& operator=(const SmartContractMessage&) = default;
    SmartContractMessage& operator=(SmartContractMessage&&) = default;
    ~SmartContractMessage() = default;

    const evmc_message& getMessage() const;

    int64_t getGas() const;

    base::Bytes getDestination() const;

    base::Bytes getSender() const;

    base::Bytes toInputData() const;

    base::Bytes getCreate2Salt() const;

    const base::Bytes& getCode() const;

    bc::Balance getValue() const;

    evmc_revision getRevision() const;

  private:
    evmc_message _message;
    base::Bytes _input_data;
    base::Bytes _contract_code;
    evmc_revision _revision;

    SmartContractMessage(evmc_revision revision);
};


class ExecuteResult
{
  public:
    ExecuteResult(evmc::result&& data);
    ExecuteResult(ExecuteResult&&) = default;
    ExecuteResult(const ExecuteResult&) = delete;
    ExecuteResult& operator=(ExecuteResult&&) = default;
    ExecuteResult& operator=(const ExecuteResult&) = delete;
    ~ExecuteResult() = default;

    bool ok() const noexcept;

    base::Bytes toOutputData() const;
    int64_t gasLeft() const;
    base::Bytes createdAddress() const;

  private:
    evmc::result _data;
};


class VM
{
  public:
    VM(const VM&) = delete;
    VM(VM&&) = default;
    VM& operator=(const VM&) = delete;
    VM& operator=(VM&&) = default;
    ~VM() = default;

    static VM load(evmc::Host& vm_host);

    ExecuteResult execute(const SmartContractMessage& msg);

  private:
    VM(evmc_vm* vm_instance_ptr, evmc::Host& vm_host);

    evmc::VM _vm;
    evmc::Host& _host;
};

} // namespace vm