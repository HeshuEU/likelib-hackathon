#include "vm.hpp"

#include "base/error.hpp"

#include <evmc/loader.h>

#include <iostream>

#include <cstring>

namespace core
{

namespace
{
    uint8_t hexToInt(char hex)
    {
        if('0' <= hex && hex <= '9') {
            return hex - '0';
        }
        else if('a' <= hex && hex <= 'f') {
            return hex - 'a' + 10;
        }
        else if('A' <= hex && hex <= 'F') {
            return hex - 'A' + 10;
        }
        else {
            RAISE_ERROR(base::InvalidArgument, "invalid hex symbol");
        }
    }

} // namespace

std::string toHex(const uint8_t* t, size_t t_size)
{
    if(t_size == 0 || t == nullptr) {
        RAISE_ERROR(base::InvalidArgument, "zero size array");
    }
    static constexpr const char HEX_DIGITS[] = "0123456789abcdef";
    // since every byte is represented by 2 hex digits, we do * 2
    const auto final_size = t_size * 2;
    std::string ret(final_size, static_cast<char>(0));
    std::size_t index = 0;
    while(index < final_size) {
        ret[index++] = HEX_DIGITS[*t >> 4];
        ret[index++] = HEX_DIGITS[*t & 0xF];
        t++;
    }
    return std::move(ret);
}

void copyData(const std::string_view& str, uint8_t* t, size_t t_size)
{
    if(str.size() != t_size) {
        RAISE_ERROR(base::InvalidArgument, "String is not correct size");
    }
    for(const auto& item: str) {
        *(t++) = item;
    }
}

void fromString(const std::string_view& str, uint8_t* t, size_t t_size)
{
    auto data = fromHex(str);
    if(data.size() != t_size) {
        RAISE_ERROR(base::InvalidArgument, "String is not correct size");
    }
    for(const auto& item: data) {
        *(t++) = item;
    }
}

std::vector<uint8_t> fromHex(const std::string_view& hex_view)
{
    if(hex_view.size() % 2 != 0) {
        RAISE_ERROR(base::InvalidArgument, "hex size is not an even number");
    }

    auto bytes_size = hex_view.size() / 2;
    std::vector<uint8_t> bytes(bytes_size);
    for(std::size_t current_symbol_index = 0; current_symbol_index < bytes_size; current_symbol_index++) {
        auto index = current_symbol_index * 2;
        auto high_part = hexToInt(hex_view[index]);
        auto low_part = hexToInt(hex_view[index + 1]);
        bytes[current_symbol_index] = (high_part << 4) + low_part;
    }

    return std::move(bytes);
}


std::string toString(const evmc::bytes32& bytes)
{
    return std::move(toHex(bytes.bytes, 32));
}

void fromString(const std::string_view& str, evmc::bytes32& bytes)
{
    fromString(str, bytes.bytes, 32);
}

std::string toString(const evmc_bytes32& bytes)
{
    return std::move(toHex(bytes.bytes, 32));
}

void fromString(const std::string_view& str, evmc_bytes32& bytes)
{
    fromString(str, bytes.bytes, 32);
}

std::string toString(const evmc::address& addr)
{
    return std::move(toHex(addr.bytes, 20));
}

void fromString(const std::string_view& str, evmc::address& address)
{
    fromString(str, address.bytes, 20);
}

std::string toString(const evmc_address& addr)
{
    return std::move(toHex(addr.bytes, 20));
}

void fromString(const std::string_view& str, evmc_address& address)
{
    fromString(str, address.bytes, 20);
}


namespace vm
{

    SmartContract::SmartContract(const std::vector<uint8_t>& contract_code, evmc_revision revision)
        : _contract_code{contract_code}, _revision(revision)
    {}

    SmartContractMessage SmartContract::createCallMessage(
        int64_t gas, evmc::address& source, evmc::address& destination, evmc::uint256be& value) const
    {
        SmartContractMessage message{_revision};
        message._contract_code = _contract_code;
        message._message.kind = evmc_call_kind::EVMC_CALL;
        message._message.depth = 0;
        message._message.gas = gas;
        message._message.sender = source;
        message._message.destination = destination;
        message._message.value = value;
        message._message.create2_salt = evmc_bytes32();
        message._message.input_data = nullptr;
        message._message.input_size = 0;
        return std::move(message);
    }

    SmartContractMessage SmartContract::createCreateMessage(int64_t gas, evmc::address& source,
        evmc::address& destination, evmc::uint256be& value, const std::vector<uint8_t>& input) const
    {
        SmartContractMessage message{_revision};
        message._contract_code = _contract_code;
        message._message.kind = evmc_call_kind::EVMC_CREATE;
        message._message.depth = 0;
        message._message.gas = gas;
        message._message.sender = source;
        message._message.destination = destination;
        message._message.value = value;
        message._message.create2_salt = evmc_bytes32();
        message._message.input_data = input.data();
        message._message.input_size = input.size();
        return std::move(message);
    }

    SmartContractMessage SmartContract::createCallAMessage(int64_t gas, evmc::address& source,
        evmc::address& destination, evmc::uint256be& value, const std::vector<uint8_t>& input) const
    {
        SmartContractMessage message{_revision};
        message._contract_code = _contract_code;
        message._message.kind = evmc_call_kind::EVMC_CALLCODE;
        message._message.depth = 0;
        message._message.gas = gas;
        message._message.sender = source;
        message._message.destination = destination;
        message._message.value = value;
        message._message.create2_salt = evmc_bytes32();
        message._message.input_data = input.data();
        message._message.input_size = input.size();
        return std::move(message);
    }

    const evmc_message& SmartContractMessage::getMessage() const
    {
        return _message;
    }

    int64_t SmartContractMessage::getGas() const
    {
        return _message.gas;
    }

    evmc_address SmartContractMessage::getDestination() const
    {
        return _message.destination;
    }

    evmc_address SmartContractMessage::getSender() const
    {
        return _message.sender;
    }

    std::vector<uint8_t> SmartContractMessage::copyData() const
    {
        std::vector<uint8_t> data(_message.input_size);
        memcpy(data.data(), _message.input_data, _message.input_size);
        return std::move(data);
    }

    evmc_bytes32 SmartContractMessage::getCreate2Salt() const
    {
        return _message.create2_salt;
    }

    const std::vector<uint8_t>& SmartContractMessage::getCode() const
    {
        return _contract_code;
    }

    evmc_uint256be SmartContractMessage::getValue() const
    {
        return _message.value;
    }

    evmc_revision SmartContractMessage::getRevision() const
    {
        return _revision;
    }


    SmartContractMessage::SmartContractMessage(evmc_revision revision)
        : _message{}, _contract_code{}, _revision{revision}
    {}


    ExecuteResult::ExecuteResult(evmc::result&& data) : _data(std::move(data))
    {}


    const evmc::result& ExecuteResult::getEvmcResult() const
    {
        return _data;
    }


    bool ExecuteResult::ok() const
    {
        return _data.status_code == evmc_status_code::EVMC_SUCCESS;
    }


    evmc_status_code ExecuteResult::getStatusCode() const
    {
        return _data.status_code;
    }


    std::vector<uint8_t> ExecuteResult::toOutputData() const
    {
        std::vector<uint8_t> res(_data.output_size);
        memcpy(res.data(), _data.output_data, _data.output_size);
        return res;
    }


    int64_t ExecuteResult::gasLeft() const
    {
        return _data.gas_left;
    }


    evmc_address ExecuteResult::createdAddress() const
    {
        return _data.create_address;
    }


    VM VM::load_from_dll(const std::string& path_to_dll, evmc::Host& vm_host)
    {
        evmc_loader_error_code load_error_code;
        auto vm_ptr = evmc_load_and_create(path_to_dll.c_str(), &load_error_code);
        if(load_error_code != EVMC_LOADER_SUCCESS || vm_ptr == nullptr) {
            RAISE_ERROR(base::InaccessibleFile, "Failed to load VM");
        }

        switch(load_error_code) {
            case EVMC_LOADER_SUCCESS:
                break;
            case EVMC_LOADER_CANNOT_OPEN:
            case EVMC_LOADER_SYMBOL_NOT_FOUND:
            case EVMC_LOADER_ABI_VERSION_MISMATCH:
            default:
                std::cerr << "Error at loading vm";
                throw std::exception();
        }

        VM vm_wrapper_instance{vm_ptr, vm_host};
        return vm_wrapper_instance;
    }


    ExecuteResult VM::execute(const SmartContractMessage& msg)
    {
        auto res = _vm.execute(_host, msg.getRevision(), msg.getMessage(), msg.getCode().data(), msg.getCode().size());
        return ExecuteResult{std::move(res)};
    }


    VM::VM(evmc_vm* vm_instance_ptr, evmc::Host& vm_host) : _vm{vm_instance_ptr}, _host{vm_host}
    {
        std::cout << "EVM name: " << _vm.name() << ", version:" << _vm.version();

        if(_vm.is_abi_compatible()) {
            std::cout << " ABI version is compatible." << std::endl;
        }

        auto vm_capabilities = _vm.get_capabilities();

        if(vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_EVM1) {
            std::cout << "EVM compatible with EVM1 instructions" << std::endl;
        }

        if(vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_EWASM) {
            std::cout << "EVM compatible with EWASM instructions" << std::endl;
        }

        if(vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_PRECOMPILES) {
            std::cout << "EVM compatible with precompiles instructions" << std::endl;
        }
    }

} // namespace vm
} // namespace core