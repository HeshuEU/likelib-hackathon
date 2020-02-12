#include "vm.hpp"
#include "error.hpp"
#include "tools.hpp"

#include "base/log.hpp"

#include <evmc/loader.h>

namespace vm
{

namespace
{

    base::Bytes getRuntimeCode(const base::Bytes& full_code)
    {
        static const std::string target{"60806040"};
        // TODO: make normal algorithm
        auto hex_code = full_code.toHex();
        auto index = hex_code.find(target, target.size());
        if(index == 0) {
            RAISE_ERROR(base::LogicError, "Not valid code");
        }
        auto sub = hex_code.substr(index, hex_code.size());
        return base::Bytes::fromHex(sub);
    }

} // namespace


SmartContract::SmartContract(const base::Bytes& contract_code) : _revision(EVMC_ISTANBUL), _contract_code{contract_code}
{}


SmartContractMessage SmartContract::createInitMessage(int64_t gas, base::Bytes& source, const base::Bytes& destination,
    const base::Bytes& value, const base::Bytes& input) const
{
    SmartContractMessage message{_revision};
    message._contract_code = _contract_code + input;
    message._message.kind = evmc_call_kind::EVMC_CALL;
    message._message.depth = 0;
    message._message.gas = gas;
    message._message.sender = toAddress(source);
    message._message.destination = toAddress(destination);
    message._message.value = toEvmcUint256(value);
    message._message.create2_salt = evmc_bytes32();
    return message;
}


SmartContractMessage SmartContract::createMessage(int64_t gas, base::Bytes& source, const base::Bytes& destination,
    const base::Bytes& value, const base::Bytes& input) const
{
    SmartContractMessage message{_revision};
    message._contract_code = getRuntimeCode(_contract_code);
    message._message.kind = evmc_call_kind::EVMC_CALL;
    message._message.depth = 0;
    message._message.gas = gas;
    message._message.sender = toAddress(source);
    message._message.destination = toAddress(destination);
    message._message.value = toEvmcUint256(value);
    message._message.create2_salt = evmc_bytes32();
    message._input_data = input;
    message._message.input_data = message._input_data.toArray();
    message._message.input_size = message._input_data.size();
    return message;
}


const evmc_message& SmartContractMessage::getMessage() const
{
    return _message;
}


int64_t SmartContractMessage::getGas() const
{
    return _message.gas;
}


base::Bytes SmartContractMessage::getDestination() const
{
    return toBytes(_message.destination);
}


base::Bytes SmartContractMessage::getSender() const
{
    return toBytes(_message.sender);
}


base::Bytes SmartContractMessage::toInputData() const
{
    return copy(_message.input_data, _message.input_size);
}


base::Bytes SmartContractMessage::getCreate2Salt() const
{
    return toBytes(_message.create2_salt);
}


const base::Bytes& SmartContractMessage::getCode() const
{
    return _contract_code;
}


base::Bytes SmartContractMessage::getValue() const
{
    return toBalance(_message.value);
}


evmc_revision SmartContractMessage::getRevision() const
{
    return _revision;
}


SmartContractMessage::SmartContractMessage(evmc_revision revision) : _message{}, _contract_code{}, _revision{revision}
{}


ExecuteResult::ExecuteResult(evmc::result&& data) : _data(std::move(data))
{}


bool ExecuteResult::ok() const noexcept
{
    return _data.status_code == evmc_status_code::EVMC_SUCCESS;
}


base::Bytes ExecuteResult::toOutputData() const
{
    return copy(_data.output_data, _data.output_size);
}


int64_t ExecuteResult::gasLeft() const
{
    return _data.gas_left;
}


base::Bytes ExecuteResult::createdAddress() const
{
    return toBytes(_data.create_address);
}


VM VM::load_from_dll(const std::string& path_to_dll, evmc::Host& vm_host)
{
    evmc_loader_error_code load_error_code;
    auto vm_ptr = evmc_load_and_create(path_to_dll.c_str(), &load_error_code);

    if(load_error_code != EVMC_LOADER_SUCCESS || vm_ptr == nullptr) {
        switch(load_error_code) {
            case EVMC_LOADER_SUCCESS:
                RAISE_ERROR(base::LogicError, "Error status is success but pointer is null");
            case EVMC_LOADER_CANNOT_OPEN:
                RAISE_ERROR(base::InaccessibleFile, "File can't be open");
            case EVMC_LOADER_SYMBOL_NOT_FOUND:
                RAISE_ERROR(VmError, "Vm dll has incorrect format");
            case EVMC_LOADER_ABI_VERSION_MISMATCH:
                RAISE_ERROR(VmError, "Invalid vm abi version");
            default:
                RAISE_ERROR(VmError, "Undefined error at loading vm");
        }
    }

    return {vm_ptr, vm_host};
}


ExecuteResult VM::execute(const SmartContractMessage& msg)
{
    auto res = _vm.execute(_host, msg.getRevision(), msg.getMessage(), msg.getCode().toArray(), msg.getCode().size());
    return ExecuteResult{std::move(res)};
}


VM::VM(evmc_vm* vm_instance_ptr, evmc::Host& vm_host) : _vm{vm_instance_ptr}, _host{vm_host}
{
    LOG_INFO << "Created EVM name: " << _vm.name() << ", version:" << _vm.version();

    if(!_vm.is_abi_compatible()) {
        RAISE_ERROR(VmError, " ABI version is incompatible.");
    }

    auto vm_capabilities = _vm.get_capabilities();

    if(vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_EVM1) {
        LOG_INFO << "EVM compatible with EVM1 instructions" << std::endl;
    }

    if(vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_EWASM) {
        LOG_INFO << "EVM compatible with EWASM instructions" << std::endl;
    }

    if(vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_PRECOMPILES) {
        LOG_INFO << "EVM compatible with precompiles instructions" << std::endl;
    }
}

} // namespace vm