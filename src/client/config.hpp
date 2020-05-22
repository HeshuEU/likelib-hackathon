#pragma once

#include "base/config.hpp"

#include <string_view>

namespace config
{
constexpr uint32_t API_VERSION = base::config::RPC_PUBLIC_API_VERSION;
constexpr std::string_view CLIENT_VERSION = "0.1";
constexpr std::string_view CONTRACT_BINARY_FILE = "compiled_code.bin";
constexpr std::string_view METADATA_JSON_FILE = "metadata.json";

} // namespace config