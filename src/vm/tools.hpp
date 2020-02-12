#pragma once

#include "base/bytes.hpp"

#include "bc/types.hpp"

#include <evmc/evmc.hpp>

namespace vm
{

base::Bytes copy(const uint8_t* t, size_t t_size);

base::Bytes toBytes(const evmc::address& addr);

evmc::address toAddress(const base::Bytes& data);

base::Bytes toBytes(const evmc::bytes32& bytes);

evmc::bytes32 toEvmcBytes32(const base::Bytes& data);

base::Bytes toBalance(evmc_uint256be value);

evmc_uint256be toEvmcUint256(const base::Bytes& data);

} // namespace vm