#pragma once

#include "base/bytes.hpp"
#include "lk/address.hpp"
#include "lk/types.hpp"

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

    SmartContractMessage createInitMessage(int64_t gas,
                                           const lk::Address& source,
                                           const lk::Address& destination,
                                           const lk::Balance& value,
                                           const base::Bytes& input) const;

    SmartContractMessage createMessage(int64_t gas,
                                       const lk::Address& source,
                                       const lk::Address& destination,
                                       const lk::Balance& value,
                                       const base::Bytes& input) const;

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
    lk::Balance getValue() const;
    evmc_revision getRevision() const;

  private:
    evmc_message _message;
    base::Bytes _input_data;
    base::Bytes _contract_code;
    evmc_revision _revision;

    SmartContractMessage(evmc_revision revision);
};


class ExecutionResult
{
  public:
    ExecutionResult() = default;
    ExecutionResult(evmc::result&& data);
    ExecutionResult(ExecutionResult&&) = default;
    ExecutionResult(const ExecutionResult&) = delete;
    ExecutionResult& operator=(ExecutionResult&&) = default;
    ExecutionResult& operator=(const ExecutionResult&) = delete;
    ~ExecutionResult() = default;

    bool ok() const noexcept;

    base::Bytes toOutputData() const;
    int64_t gasLeft() const;
    base::Bytes createdAddress() const;

    evmc::result getResult() noexcept;

  private:
    std::optional<evmc::result> _data;
};


class Vm
{
  public:
    Vm(const Vm&) = delete;
    Vm(Vm&&) = default;
    Vm& operator=(const Vm&) = delete;
    Vm& operator=(Vm&&) = default;
    ~Vm() = default;

    static Vm load(evmc::Host& vm_host);

    ExecutionResult execute(const SmartContractMessage& msg);

  private:
    Vm(evmc_vm* vm_instance_ptr, evmc::Host& vm_host);

    evmc::VM _vm;
    evmc::Host& _host;
};

} // namespace vm