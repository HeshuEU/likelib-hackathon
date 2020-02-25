#include "host.hpp"

namespace vm
{


HostImplementation::HostImplementation()
{}


bool HostImplementation::account_exists(const evmc::address& addr) const noexcept
{
    return !(reinterpret_cast<long long>(&addr) % 2);
}


evmc::bytes32 HostImplementation::get_storage(const evmc::address& addr, const evmc::bytes32& key) const noexcept
{}


evmc_storage_status HostImplementation::set_storage(
    const evmc::address& addr, const evmc::bytes32& key, const evmc::bytes32& value) noexcept
{}


evmc::uint256be HostImplementation::get_balance(const evmc::address& addr) const noexcept
{}


size_t HostImplementation::get_code_size(const evmc::address& addr) const noexcept
{}


evmc::bytes32 HostImplementation::get_code_hash(const evmc::address& addr) const noexcept
{}


size_t HostImplementation::copy_code(
    const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const noexcept
{}


void HostImplementation::selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept
{}


evmc::result HostImplementation::call(const evmc_message& msg) noexcept
{}


evmc_tx_context HostImplementation::get_tx_context() const noexcept
{}


evmc::bytes32 HostImplementation::get_block_hash(int64_t block_number) const noexcept
{}


void HostImplementation::emit_log(const evmc::address& addr, const uint8_t* data, size_t data_size,
    const evmc::bytes32 topics[], size_t num_topics) noexcept
{}

} // namespace vm