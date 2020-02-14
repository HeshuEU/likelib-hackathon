#include <boost/test/unit_test.hpp>

#include <vm/vm.hpp>
#include <vm/tools.hpp>
#include <vm/messages.hpp>

#include <filesystem>
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
        auto f = _storage.find(key);
        if(f != _storage.end()) {
            f->second = value;
            return evmc_storage_status::EVMC_STORAGE_MODIFIED;
        }

        _storage.insert({key, value});
        return evmc_storage_status::EVMC_STORAGE_ADDED;
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


BOOST_AUTO_TEST_CASE(vm_base, *boost::unit_test::disabled())
{
    const char* source_code = R"V0G0N(
pragma solidity >=0.4.16 <0.7.0;

contract Foo {
    uint32 public _x;

    constructor(uint32 x) public payable{
        _x = x;
    }

    function bar(uint32 x) public {
        _x = x;
    }

    function baz(uint32 x, bool y) public pure returns (bool r) {r = x > 32 || y;}

    function sam(bytes memory, bool, uint[] memory) public pure {}
}
)V0G0N";

    std::filesystem::path code_file_path = "vm_base.sol";

    std::ofstream file_descriptor(code_file_path);
    file_descriptor << source_code;
    file_descriptor.close();

    HostImplementation host{};
    auto vm_instance = vm::VM::load(host);

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

    auto target_value_1 = 1u;
    auto init_message = contract.createInitMessage(100000, source, destination, 1, vm::encode(target_value_1));
    auto res1 = vm_instance.execute(init_message);
    BOOST_CHECK(res1.ok());

    auto input_for_message_1 = base::Bytes::fromHex("29809703"); // TODO: remove
    auto message1 = contract.createMessage(10000, source, destination, 0, input_for_message_1);
    auto res2 = vm_instance.execute(message1);
    BOOST_CHECK(res2.ok());

    BOOST_CHECK_EQUAL(res2.toOutputData(), vm::encode(target_value_1));

    auto target_value_2 = 69u;

    auto input_for_message_2 = base::Bytes::fromHex("bd70b447") + vm::encode(target_value_2);
    auto message2 = contract.createMessage(10000, source, destination, 0, input_for_message_2);
    auto res3 = vm_instance.execute(message2);
    BOOST_CHECK(res3.ok());

    auto input_for_message_3 = base::Bytes::fromHex("29809703"); // TODO: remove
    auto message3 = contract.createMessage(10000, source, destination, 0, input_for_message_3);
    auto res4 = vm_instance.execute(message3);
    BOOST_CHECK(res3.ok());

    BOOST_CHECK_EQUAL(res4.toOutputData(), vm::encode(target_value_2));

    std::filesystem::remove(code_file_path);
}


BOOST_AUTO_TEST_CASE(vm_dynamic_params, *boost::unit_test::disabled())
{
    const char* source_code = R"V0G0N(
pragma solidity >=0.4.16 <0.7.0;

contract Foo {

    string public message;

    constructor(string memory initMessage) public {
        message = initMessage;
    }

    function update(string memory newMessage) public {
        message = newMessage;
    }
}
    )V0G0N";

    std::filesystem::path code_file_path = "vm_dynamic_params.sol";

    std::ofstream file_descriptor(code_file_path);
    file_descriptor << source_code;
    file_descriptor.close();

    HostImplementation host{};
    auto vm_instance = vm::VM::load(host);

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

    std::string target_value_1 = "one";

    auto init_message =
        contract.createInitMessage(1000000, source, destination, 0, vm::encode(32u) + vm::encode(target_value_1));
    auto res1 = vm_instance.execute(init_message);
    BOOST_CHECK(res1.ok());

    auto message1 = contract.createMessage(1000000, source, destination, 0, base::Bytes::fromHex("e21f37ce"));
    auto res2 = vm_instance.execute(message1);
    BOOST_CHECK(res2.ok());

    BOOST_CHECK_EQUAL(vm::getStringArg(0, res2.toOutputData()), target_value_1);

    std::string target_value_2 = "Hello, world";

    auto message2 = contract.createMessage(1000000, source, destination, 0,
        base::Bytes::fromHex("3d7403a3") + vm::encode(32u) + vm::encode(target_value_2));
    auto res3 = vm_instance.execute(message2);
    BOOST_CHECK(res3.ok());

    auto message3 = contract.createMessage(1000000, source, destination, 0, base::Bytes::fromHex("e21f37ce"));
    auto res4 = vm_instance.execute(message3);
    BOOST_CHECK(res3.ok());

    BOOST_CHECK_EQUAL(vm::getStringArg(0, res4.toOutputData()), target_value_2);

    std::filesystem::remove(code_file_path);
}