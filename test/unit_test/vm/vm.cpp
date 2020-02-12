#include <boost/test/unit_test.hpp>

#include <vm/vm.hpp>
#include <vm/tools.hpp>
#include <vm/messages.hpp>

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
        std::cout << "account_exists method call: address[ " << vm::toBytes(addr) << "].\n";
        return false;
    }

    evmc::bytes32 get_storage(const evmc::address& addr, const evmc::bytes32& key) const noexcept override
    {
        std::cout << "get_storage method call: address[" << vm::toBytes(addr) << "], key[" << vm::toBytes(key)
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
        std::cout << "set_storage method call: address[" << vm::toBytes(addr).toHex() << "], key[" << vm::toBytes(key)
                  << "], value[" << vm::toBytes(value) << "].\n";
        auto status = evmc_storage_status::EVMC_STORAGE_ADDED;
        if(_storage.find(key) != _storage.end()) {
            status = evmc_storage_status::EVMC_STORAGE_MODIFIED;
        }

        _storage.insert({key, value});
        return status;
    }

    evmc::uint256be get_balance(const evmc::address& addr) const noexcept override
    {
        std::cout << "get_balance method call: address[" << vm::toBytes(addr) << "].\n";
        return evmc::uint256be();
    }

    size_t get_code_size(const evmc::address& addr) const noexcept override
    {
        std::cout << "get_code_size method call: address: " << vm::toBytes(addr) << "].\n";
        return 0;
    }

    evmc::bytes32 get_code_hash(const evmc::address& addr) const noexcept override
    {
        std::cout << "get_code_hash call method call: " << vm::toBytes(addr) << "].\n";
        return evmc::bytes32();
    }

    size_t copy_code(const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const
        noexcept override
    {
        std::cout << "copy_code call method call: " << vm::toBytes(addr)
                  << ", code_offset: " << std::to_string(code_offset)
                  << ", buffer_size: " << std::to_string(buffer_size) << "].\n";
        return 0;
    }

    void selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept override
    {
        std::cout << "selfdestruct method call: address[" << vm::toBytes(addr) << "], beneficiary["
                  << vm::toBytes(beneficiary) << "].\n";
    }

    evmc::result call(const evmc_message& msg) noexcept override
    {
        std::cout << "call method call: snd[" << vm::toBytes(msg.sender) << "], dest[" << vm::toBytes(msg.destination)
                  << "].\n";
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
        std::cout << "get_block_hash method call: block_number[" << std::to_string(block_number) << "].\n";
        return evmc::bytes32();
    }

    void emit_log(const evmc::address& addr, const uint8_t* data, size_t data_size, const evmc::bytes32 topics[],
        size_t num_topics) noexcept override
    {
        std::cout << "emit_log method call: address[" << vm::toBytes(addr) << "], topics_number["
                  << std::to_string(num_topics) << "].\n";
    }
};

BOOST_AUTO_TEST_CASE(vm_base)
{
    //    const auto path_to_dll =
    //            "/home/siarhei_sadouski/Documents/External projects/evmone/cmake-build-debug/lib/libevmone.so.0.4.0";
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
    auto vm_instance = vm::VM::load_from_dll(path_to_dll, host);

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
    //////////////////////////////////////////////////

    auto source = base::Bytes::fromHex("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaf");
    auto destination = base::Bytes::fromHex("ffffffffffffffffffffffffffffffffffffaaff");

    vm::SmartContract contract{target_code};

    int64_t gas_for_init = 1000000;
    auto init_value = base::Bytes::fromHex("0000000000000000000000000000000000000000000000000000000000000001");
    auto init_input = base::Bytes::fromHex("0000000000000000000000000000000000000000000000000000000000000001");

    auto init_message = contract.createInitMessage(gas_for_init, source, destination, init_value, init_input);
    auto res1 = vm_instance.execute(init_message);
    BOOST_CHECK(res1.ok());

    int64_t gas_for_message_1 = 1000000;
    auto value_for_message_1 = base::Bytes::fromHex("0000000000000000000000000000000000000000000000000000000000000000");
    auto input_for_message_1 =
        base::Bytes::fromHex("bd70b4470000000000000000000000000000000000000000000000000000000000000045");

    auto message1 =
        contract.createMessage(gas_for_message_1, source, destination, value_for_message_1, input_for_message_1);
    auto res2 = vm_instance.execute(message1);
    BOOST_CHECK(res2.ok());

    int64_t gas_for_message_2 = 1000000;
    auto value_for_message_2 = base::Bytes::fromHex("0000000000000000000000000000000000000000000000000000000000000000");
    auto input_for_message_2 = base::Bytes::fromHex("29809703");

    auto message2 =
        contract.createMessage(gas_for_message_2, source, destination, value_for_message_2, input_for_message_2);
    auto res3 = vm_instance.execute(message2);
    BOOST_CHECK(res3.ok());

    std::filesystem::remove(code_file_path);
}
