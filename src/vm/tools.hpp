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

bc::Balance toBalance(evmc_uint256be value);

evmc_uint256be toEvmcUint256(const bc::Balance& balance);

std::string getStringArg(size_t position, const base::Bytes& data);

base::Bytes encode(const std::string& str);

std::string decodeAsString(const base::Bytes& data);

base::Bytes encode(size_t value);

size_t decodeAsSizeT(const base::Bytes& data);

base::Bytes encode(uint32_t value);

base::Bytes encode(uint16_t value);

base::Bytes encode(uint8_t value);

} // namespace vm