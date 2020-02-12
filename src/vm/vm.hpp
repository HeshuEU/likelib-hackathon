#pragma once

#include <evmc/evmc.hpp>

#include <evmc/evmc.h>


namespace core
{

std::string toHex(const uint8_t* t, size_t t_size);

void copyData(const std::string_view& str, uint8_t* t, size_t t_size);

void fromString(const std::string_view& str, uint8_t* t, size_t t_size);

std::vector<uint8_t> fromHex(const std::string_view& hex_view);

std::string toString(const evmc::bytes32& bytes);

void fromString(const std::string_view& str, evmc::bytes32& bytes);

std::string toString(const evmc_bytes32& bytes);

void fromString(const std::string_view& str, evmc_bytes32& bytes);

std::string toString(const evmc::address& addr);

void fromString(const std::string_view& str, evmc::address& address);

std::string toString(const evmc_address& addr);

void fromString(const std::string_view& str, evmc_address& address);

template<typename T>
std::string toString(const T& t)
{
    return std::move(std::to_string(t));
}

namespace vm
{

    class SmartContractMessage;

    class SmartContract
    {
      public:
        SmartContract(const std::vector<uint8_t>& contract_code, evmc_revision revision);
        SmartContract(const SmartContract&) = default;
        SmartContract(SmartContract&&) = default;
        SmartContract& operator=(const SmartContract&) = default;
        SmartContract& operator=(SmartContract&&) = default;
        ~SmartContract() = default;

        SmartContractMessage createCallMessage(
            int64_t gas, evmc::address& source, evmc::address& destination, evmc::uint256be& value) const;
        SmartContractMessage createCreateMessage(int64_t gas, evmc::address& source, evmc::address& destination,
            evmc::uint256be& value, const std::vector<uint8_t>& input) const;
        SmartContractMessage createCallAMessage(int64_t gas, evmc::address& source, evmc::address& destination,
            evmc::uint256be& value, const std::vector<uint8_t>& input) const;

      private:
        std::vector<uint8_t> _contract_code;
        evmc_revision _revision;
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

        evmc_address getDestination() const;

        evmc_address getSender() const;

        std::vector<uint8_t> copyData() const;

        evmc_bytes32 getCreate2Salt() const;

        const std::vector<uint8_t>& getCode() const;

        evmc_uint256be getValue() const;

        evmc_revision getRevision() const;

      private:
        evmc_message _message;
        std::vector<uint8_t> _contract_code;
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

        const evmc::result& getEvmcResult() const;

        bool ok() const;
        evmc_status_code getStatusCode() const;

        std::vector<uint8_t> toOutputData() const;
        int64_t gasLeft() const;
        evmc_address createdAddress() const;

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

        static VM load_from_dll(const std::string& path_to_dll, evmc::Host& vm_host);

        ExecuteResult execute(const SmartContractMessage& msg);

      private:
        VM(evmc_vm* vm_instance_ptr, evmc::Host& vm_host);

        evmc::Host& _host;
        evmc::VM _vm;
    };

} // namespace vm
} // namespace core