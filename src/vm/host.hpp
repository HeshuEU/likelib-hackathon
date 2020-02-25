#pragma once

#include "evmc/evmc.hpp"

namespace vm
{

class HostImplementation : public evmc::Host
{
  public:
    HostImplementation();

    ~HostImplementation() noexcept override = default;

    bool account_exists(const evmc::address& addr) const noexcept override;

    evmc::bytes32 get_storage(const evmc::address& addr, const evmc::bytes32& key) const noexcept override;

    evmc_storage_status set_storage(
        const evmc::address& addr, const evmc::bytes32& key, const evmc::bytes32& value) noexcept override;

    evmc::uint256be get_balance(const evmc::address& addr) const noexcept override;

    size_t get_code_size(const evmc::address& addr) const noexcept override;

    evmc::bytes32 get_code_hash(const evmc::address& addr) const noexcept override;

    size_t copy_code(const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const
        noexcept override;

    void selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept override;

    evmc::result call(const evmc_message& msg) noexcept override;

    evmc_tx_context get_tx_context() const noexcept override;

    evmc::bytes32 get_block_hash(int64_t block_number) const noexcept override;

    void emit_log(const evmc::address& addr, const uint8_t* data, size_t data_size, const evmc::bytes32 topics[],
        size_t num_topics) noexcept override;

  private:
};

} // namespace vm