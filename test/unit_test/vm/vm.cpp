#include <boost/test/unit_test.hpp>

#include <vm/vm.hpp>
#include <vm/solc.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>


class HostImplementation : public evmc::Host
{
    std::map<evmc::bytes32, evmc::bytes32> _storage;

  public:
    ~HostImplementation() noexcept override = default;

    bool account_exists(const evmc::address& addr) const noexcept override
    {
        std::cout << "account_exists method call: address[ " << core::toString(addr) << "].\n";
        return false;
    }

    evmc::bytes32 get_storage(const evmc::address& addr, const evmc::bytes32& key) const noexcept override
    {
        std::cout << "get_storage method call: address[" << core::toString(addr) << "], key[" << core::toString(key)
                  << "].\n";
        auto it = _storage.find(key);
        if(it != _storage.end()) {
            auto val = it->second;
            return val;
        }
        return evmc::bytes32();
    }

    evmc_storage_status set_storage(
        const evmc::address& addr, const evmc::bytes32& key, const evmc::bytes32& value) noexcept override
    {
        std::cout << "set_storage method call: address[" << core::toString(addr) << "], key[" << core::toString(key)
                  << "], value[" << core::toString(value) << "].\n";
        auto status = evmc_storage_status::EVMC_STORAGE_ADDED;
        if(_storage.find(key) != _storage.end()) {
            status = evmc_storage_status::EVMC_STORAGE_MODIFIED;
        }

        _storage.insert({key, value});
        return status;
    }

    evmc::uint256be get_balance(const evmc::address& addr) const noexcept override
    {
        std::cout << "get_balance method call: address[" << core::toString(addr) << "].\n";
        return evmc::uint256be();
    }

    size_t get_code_size(const evmc::address& addr) const noexcept override
    {
        std::cout << "get_code_size method call: address: " << core::toString(addr) << "].\n";
        return 0;
    }

    evmc::bytes32 get_code_hash(const evmc::address& addr) const noexcept override
    {
        std::cout << "get_code_hash call method call: " << core::toString(addr) << "].\n";
        return evmc::bytes32();
    }

    size_t copy_code(const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const
        noexcept override
    {
        std::cout << "copy_code call method call: " << core::toString(addr)
                  << ", code_offset: " << core::toString(code_offset)
                  << ", buffer_size: " << core::toString(buffer_size) << "].\n";
        return 0;
    }

    void selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept override
    {
        std::cout << "selfdestruct method call: address[" << core::toString(addr) << "], beneficiary["
                  << core::toString(beneficiary) << "].\n";
    }

    evmc::result call(const evmc_message& msg) noexcept override
    {
        std::cout << "call method call: snd[" << core::toString(msg.sender) << "], dest["
                  << core::toString(msg.destination) << "].\n";
        evmc::result res{evmc_status_code::EVMC_SUCCESS, 0, nullptr, 0};
        return res;
    }

    evmc_tx_context get_tx_context() const noexcept override
    {
        std::cout << "get_tx_context method call \n";
        return evmc_tx_context();
    }

    evmc::bytes32 get_block_hash(int64_t block_number) const noexcept override
    {
        std::cout << "get_block_hash method call: block_number[" << core::toString(block_number) << "].\n";
        return evmc::bytes32();
    }

    void emit_log(const evmc::address& addr, const uint8_t* data, size_t data_size, const evmc::bytes32 topics[],
        size_t num_topics) noexcept override
    {
        auto parsed_data = core::toHex(data, data_size);
        std::cout << "emit_log method call: address[" << core::toString(addr) << "], data[" << parsed_data
                  << "], topics_number[" << core::toString(num_topics) << "].\n";
    }
};

BOOST_AUTO_TEST_CASE(vm_base)
{
    //    const auto path_to_dll =
    //        "/home/siarhei_sadouski/Documents/External projects/evmone/cmake-build-debug/lib/libevmone.so.0.4.0";
    const auto path_to_dll = "/home/siarhei_sadouski/Documents/vm_so_files/origin/libevmone.so.0.4.0";

    auto source_code = "pragma solidity >=0.4.16 <0.7.0;\n"
                       "\n"
                       "\n"
                       "contract Foo {\n"
                       "    uint32 public _x;\n"
                       "\n"
                       "    constructor(uint32 x) public payable{\n"
                       "        _x = x;\n"
                       "    }\n"
                       "\n"
                       "    function bar(uint32 x) public {\n"
                       "        _x = x;\n"
                       "    }\n"
                       "\n"
                       "    function baz(uint32 x, bool y) public pure returns (bool r) {r = x > 32 || y;}\n"
                       "\n"
                       "    function sam(bytes memory, bool, uint[] memory) public pure {}\n"
                       "}\n";

    std::filesystem::path code_file_path = "vm_base.sol";

    std::ofstream file_descriptor(code_file_path);
    file_descriptor << source_code;
    file_descriptor.close();

    HostImplementation host{};
    auto vm_instance = core::vm::VM::load_from_dll(path_to_dll, host);

    vm::Solc solc;

    auto contracts = solc.compile(code_file_path.string());
    if(!contracts) {
        BOOST_CHECK(false);
    }

    auto target_contract = contracts->front();
    for(const auto& sign: target_contract.getSignatures()) {
        std::cout << sign.first.toHex() << " | " << sign.second << std::endl;
    }

    auto target_code = target_contract.getFullCode();

    std::cout << target_code.toHex() << std::endl;

    auto code1 = core::fromHex(
        "60806040526040516103583803806103588339818101604052602081101561002657600080fd5b8101908080519060200190929190505050806000806101000a81548163ffffffff021916908363ffffffff160217905550506102f1806100676000396000f3fe608060405234801561001057600080fd5b506004361061004c5760003560e01c80632980970314610051578063a5643bf21461007b578063bd70b447146101d6578063cdcd77c01461020a575b600080fd5b610059610262565b604051808263ffffffff1663ffffffff16815260200191505060405180910390f35b6101d46004803603606081101561009157600080fd5b81019080803590602001906401000000008111156100ae57600080fd5b8201836020820111156100c057600080fd5b803590602001918460018302840111640100000000831117156100e257600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f8201169050808301925050505050505091929192908035151590602001909291908035906020019064010000000081111561015157600080fd5b82018360208201111561016357600080fd5b8035906020019184602083028401116401000000008311171561018557600080fd5b919080806020026020016040519081016040528093929190818152602001838360200280828437600081840152601f19601f820116905080830192505050505050509192919290505050610277565b005b610208600480360360208110156101ec57600080fd5b81019080803563ffffffff16906020019092919050505061027c565b005b6102486004803603604081101561022057600080fd5b81019080803563ffffffff16906020019092919080351515906020019092919050505061029f565b604051808215151515815260200191505060405180910390f35b6000809054906101000a900463ffffffff1681565b505050565b806000806101000a81548163ffffffff021916908363ffffffff16021790555050565b600060208363ffffffff1611806102b35750815b90509291505056fea26469706673582212203b136ce071276f535558f628c05372b85cd3e28e401347311294618aa2e0ba5c64736f6c634300060200330000000000000000000000000000000000000000000000000000000000000001");
    core::vm::SmartContract contract1{code1, evmc_revision::EVMC_ISTANBUL};

    int64_t gas{100000000};
    evmc::address source{};
    core::fromString("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaf", source);
    evmc::address destination{};
    core::fromString("ffffffffffffffffffffffffffffffffffffaaff", destination);
    evmc::uint256be value{};
    core::fromString("0000000000000000000000000000000000000000000000000000000000000001", value);

    auto input = core::fromHex("");
    auto message = contract1.createCallAMessage(gas, source, destination, value, input);
    auto res = vm_instance.execute(message);
    if(res.ok()) {
        std::cout << "Test0 init passed" << std::endl;
    }
    else {
        std::cout << "Test0 init failed: " << std::to_string(res.getStatusCode()) << std::endl;
        return;
    }

    evmc::uint256be value2{};
    core::fromString("0000000000000000000000000000000000000000000000000000000000000000", value2);

    auto code2 = core::fromHex(
        "608060405234801561001057600080fd5b506004361061004c5760003560e01c80632980970314610051578063a5643bf21461007b578063bd70b447146101d6578063cdcd77c01461020a575b600080fd5b610059610262565b604051808263ffffffff1663ffffffff16815260200191505060405180910390f35b6101d46004803603606081101561009157600080fd5b81019080803590602001906401000000008111156100ae57600080fd5b8201836020820111156100c057600080fd5b803590602001918460018302840111640100000000831117156100e257600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f8201169050808301925050505050505091929192908035151590602001909291908035906020019064010000000081111561015157600080fd5b82018360208201111561016357600080fd5b8035906020019184602083028401116401000000008311171561018557600080fd5b919080806020026020016040519081016040528093929190818152602001838360200280828437600081840152601f19601f820116905080830192505050505050509192919290505050610277565b005b610208600480360360208110156101ec57600080fd5b81019080803563ffffffff16906020019092919050505061027c565b005b6102486004803603604081101561022057600080fd5b81019080803563ffffffff16906020019092919080351515906020019092919050505061029f565b604051808215151515815260200191505060405180910390f35b6000809054906101000a900463ffffffff1681565b505050565b806000806101000a81548163ffffffff021916908363ffffffff16021790555050565b600060208363ffffffff1611806102b35750815b90509291505056fea26469706673582212203b136ce071276f535558f628c05372b85cd3e28e401347311294618aa2e0ba5c64736f6c63430006020033");
    core::vm::SmartContract contract2{code2, evmc_revision::EVMC_ISTANBUL};

    auto input2 = core::fromHex("bd70b4470000000000000000000000000000000000000000000000000000000000000045");
    auto message2 = contract2.createCallAMessage(gas, source, destination, value2, input2);
    auto res2 = vm_instance.execute(message2);
    if(res2.ok()) {
        std::cout << "Test0 message passed" << std::endl;
    }
    else {
        std::cout << "Test0 message failed: " << std::to_string(res2.getStatusCode()) << std::endl;
        return;
    }

    auto input3 = core::fromHex("29809703");
    auto message3 = contract2.createCallAMessage(gas, source, destination, value2, input3);
    auto res3 = vm_instance.execute(message3);
    if(res3.ok()) {
        std::cout << "Test0 message3 passed" << std::endl;
    }
    else {
        std::cout << "Test0 message3 failed: " << std::to_string(res3.getStatusCode()) << std::endl;
        return;
    }



//    std::filesystem::remove(code_file_path);
}
