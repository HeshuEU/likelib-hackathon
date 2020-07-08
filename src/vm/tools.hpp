#pragma once

#include "core/address.hpp"
#include "core/types.hpp"

#include "base/bytes.hpp"

#include <evmc/evmc.hpp>

#include <boost/filesystem.hpp>

#include <optional>

namespace vm
{

base::Bytes copy(const uint8_t* t, size_t t_size);

base::Bytes toBytes(const evmc::address& addr);

evmc::address toAddress(const base::Bytes& data);

base::Bytes toBytes(const evmc::bytes32& bytes);

evmc::bytes32 toEvmcBytes32(const base::Bytes& data);

evmc::bytes32 toEvmcBytes32(const base::FixedBytes<32>& data);

lk::Balance toBalance(evmc_uint256be value);

evmc_uint256be toEvmcUint256(const lk::Balance& balance);

std::string getStringArg(size_t position, const base::Bytes& data);

base::Bytes encode(const std::string& str);

std::string decodeAsString(const base::Bytes& data);

base::Bytes encode(size_t value);

size_t decodeAsSizeT(const base::Bytes& data);

base::Bytes encode(uint32_t value);

base::Bytes encode(uint16_t value);

base::Bytes encode(uint8_t value);

lk::Address toNativeAddress(const evmc::address& addr);

evmc::address toEthAddress(const lk::Address& address);

base::Keccak256 methodHash(const boost::property_tree::ptree& method_abi);

std::optional<base::Bytes> encodeCall(const std::filesystem::path& path_to_code_folder, const std::string& call);

std::optional<std::string> decodeOutput(const std::filesystem::path& path_to_code_folder, const std::string& output);

std::string callPython(std::vector<std::string>& args);

std::string encodeMessage(const std::string& contract_path, const std::string& data);

std::string decodeMessage(const std::string& contract_path, const std::string& data);

} // namespace vm