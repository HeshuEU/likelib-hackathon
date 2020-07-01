#include <boost/test/unit_test.hpp>

#include <vm/tools.hpp>
#include <vm/vm.hpp>

#include <filesystem>
#include <iostream>
#include <map>

class HostImplementation : public evmc::Host
{
    std::map<evmc::bytes32, evmc::bytes32> _storage;

  public:
    ~HostImplementation() noexcept override = default;

    bool account_exists([[maybe_unused]] const evmc::address& addr) const noexcept override
    {
        //        std::cout << "account_exists method call: address[ " << vm::toBytes(addr) << "].\n";
        return false;
    }

    evmc::bytes32 get_storage([[maybe_unused]] const evmc::address& addr, const evmc::bytes32& key) const
      noexcept override
    {
        //        std::cout << "get_storage method call: address[" << vm::toBytes(addr) << "], key[" << vm::toBytes(key)
        //                  << "].\n";
        auto it = _storage.find(key);
        if (it != _storage.end()) {
            auto val = it->second;
            return val;
        }
        return evmc::bytes32();
    }

    evmc_storage_status set_storage([[maybe_unused]] const evmc::address& addr,
                                    const evmc::bytes32& key,
                                    const evmc::bytes32& value) noexcept override
    {
        //        std::cout << "set_storage method call: address[" << base::toHex(vm::toBytes(addr)) << "], key["
        //                  << vm::toBytes(key) << "], value[" << vm::toBytes(value) << "].\n";
        auto f = _storage.find(key);
        if (f != _storage.end()) {
            f->second = value;
            return evmc_storage_status::EVMC_STORAGE_MODIFIED;
        }

        _storage.insert({ key, value });
        return evmc_storage_status::EVMC_STORAGE_ADDED;
    }

    evmc::uint256be get_balance([[maybe_unused]] const evmc::address& addr) const noexcept override
    {
        //        std::cout << "get_balance method call: address[" << vm::toBytes(addr) << "].\n";
        return evmc::uint256be();
    }

    size_t get_code_size([[maybe_unused]] const evmc::address& addr) const noexcept override
    {
        //        std::cout << "get_code_size method call: address: " << vm::toBytes(addr) << "].\n";
        return 0;
    }

    evmc::bytes32 get_code_hash([[maybe_unused]] const evmc::address& addr) const noexcept override
    {
        //        std::cout << "get_code_hash call method call: " << vm::toBytes(addr) << "].\n";
        return evmc::bytes32();
    }

    size_t copy_code([[maybe_unused]] const evmc::address& addr,
                     [[maybe_unused]] size_t code_offset,
                     [[maybe_unused]] uint8_t* /*buffer_data*/,
                     [[maybe_unused]] size_t buffer_size) const noexcept override
    {
        //        std::cout << "copy_code call method call: " << vm::toBytes(addr)
        //                  << ", code_offset: " << std::to_string(code_offset)
        //                  << ", buffer_size: " << std::to_string(buffer_size) << "].\n";
        return 0;
    }

    void selfdestruct([[maybe_unused]] const evmc::address& addr,
                      [[maybe_unused]] const evmc::address& beneficiary) noexcept override
    {
        //        std::cout << "selfdestruct method call: address[" << vm::toBytes(addr) << "], beneficiary["
        //                  << vm::toBytes(beneficiary) << "].\n";
    }

    evmc::result call([[maybe_unused]] const evmc_message& msg) noexcept override
    {
        //        std::cout << "call method call: snd[" << vm::toBytes(msg.sender) << "], dest[" <<
        //        vm::toBytes(msg.destination)
        //                  << "].\n";
        evmc::result res{ evmc_status_code::EVMC_SUCCESS, 0, nullptr, 0 };
        return res;
    }

    evmc_tx_context get_tx_context() const noexcept override
    {
        //        std::cout << "get_tx_context method call \n";
        return evmc_tx_context();
    }

    evmc::bytes32 get_block_hash([[maybe_unused]] int64_t block_number) const noexcept override
    {
        //        std::cout << "get_block_hash method call: block_number[" << std::to_string(block_number) << "].\n";
        return evmc::bytes32();
    }

    void emit_log([[maybe_unused]] const evmc::address& addr,
                  [[maybe_unused]] const uint8_t* /*data*/,
                  [[maybe_unused]] size_t /*data_size*/,
                  [[maybe_unused]] const evmc::bytes32[] /*topics*/,
                  [[maybe_unused]] size_t num_topics) noexcept override
    {
        //        std::cout << "emit_log method call: address[" << vm::toBytes(addr) << "], topics_number["
        //                  << std::to_string(num_topics) << "].\n";
    }
};


BOOST_AUTO_TEST_CASE(vm_base)
{
    const char* source_code = R"raw(
pragma solidity >=0.4.0 <0.7.0;

contract SimpleStorage {
    uint storedData;

    constructor(uint x) public{
        storedData = x;
    }

    function set(uint x) public {
        storedData = x;
    }

    function get() public view returns (uint stored_data) {
        return storedData;
    }
}

)raw";

    std::filesystem::path code_file_path = "vm_base.sol";

    std::ofstream file_descriptor(code_file_path);
    file_descriptor << source_code;
    file_descriptor.close();

    auto contracts = vm::compile(code_file_path.string());
    if (!contracts) {
        BOOST_CHECK(false);
    }

    const auto& target_contract = contracts->front();
    //////////////////////////////////////////////////

    auto source = lk::Address{ base::fromHex<base::Bytes>("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaf") };
    auto destination = lk::Address{ base::fromHex<base::Bytes>("ffffffffffffffffffffffffffffffffffffaaff") };

    HostImplementation eth_host{};
    auto vm_instance = vm::load();

    auto target_value_1 = 1u;
    auto message_1_data = vm::encode(target_value_1);

    evmc_message message_1{};
    message_1.kind = evmc_call_kind::EVMC_CALL;
    message_1.flags = 0;
    message_1.depth = 0;
    message_1.gas = 100000;
    message_1.sender = vm::toEthAddress(source);
    message_1.destination = vm::toEthAddress(destination);
    message_1.value = vm::toEvmcUint256(0);

    auto init_code = target_contract.code + message_1_data;
    auto res1 =
      vm_instance.execute(eth_host, evmc_revision::EVMC_ISTANBUL, message_1, init_code.getData(), init_code.size());
    BOOST_CHECK(res1.status_code == evmc_status_code::EVMC_SUCCESS);
    auto target_code = vm::copy(res1.output_data, res1.output_size);

    auto message_2_data = base::fromHex<base::Bytes>("6d4ce63c");

    evmc_message message_2{};
    message_2.kind = evmc_call_kind::EVMC_CALL;
    message_2.flags = 0;
    message_2.depth = 0;
    message_2.gas = 100000;
    message_2.sender = vm::toEthAddress(source);
    message_2.destination = vm::toEthAddress(destination);
    message_2.value = vm::toEvmcUint256(0);
    message_2.input_data = message_2_data.getData();
    message_2.input_size = message_2_data.size();

    auto res2 =
      vm_instance.execute(eth_host, evmc_revision::EVMC_ISTANBUL, message_2, target_code.getData(), target_code.size());
    BOOST_CHECK(res2.status_code == evmc_status_code::EVMC_SUCCESS);

    auto message_2_output = vm::copy(res2.output_data, res2.output_size);
    BOOST_CHECK_EQUAL(message_2_output, vm::encode(target_value_1));

    auto target_value_2 = 69u;

    auto message_3_data = base::fromHex<base::Bytes>("60fe47b1") + vm::encode(target_value_2);
    evmc_message message_3{};
    message_3.kind = evmc_call_kind::EVMC_CALL;
    message_3.flags = 0;
    message_3.depth = 0;
    message_3.gas = 100000;
    message_3.sender = vm::toEthAddress(source);
    message_3.destination = vm::toEthAddress(destination);
    message_3.value = vm::toEvmcUint256(0);
    message_3.input_data = message_3_data.getData();
    message_3.input_size = message_3_data.size();

    auto res3 =
      vm_instance.execute(eth_host, evmc_revision::EVMC_ISTANBUL, message_3, target_code.getData(), target_code.size());
    BOOST_CHECK(res3.status_code == evmc_status_code::EVMC_SUCCESS);

    auto message_4_data = base::fromHex<base::Bytes>("6d4ce63c");
    evmc_message message_4{};
    message_4.kind = evmc_call_kind::EVMC_CALL;
    message_4.flags = 0;
    message_4.depth = 0;
    message_4.gas = 100000;
    message_4.sender = vm::toEthAddress(source);
    message_4.destination = vm::toEthAddress(destination);
    message_4.value = vm::toEvmcUint256(0);
    message_4.input_data = message_4_data.getData();
    message_4.input_size = message_4_data.size();

    auto res4 =
      vm_instance.execute(eth_host, evmc_revision::EVMC_ISTANBUL, message_4, target_code.getData(), target_code.size());
    BOOST_CHECK(res3.status_code == evmc_status_code::EVMC_SUCCESS);

    auto message_4_output = vm::copy(res4.output_data, res4.output_size);
    BOOST_CHECK_EQUAL(message_4_output, vm::encode(target_value_2));

    std::filesystem::remove(code_file_path);
}
