#pragma once

#include <base/config.hpp>

#include <string_view>

namespace config
{
constexpr uint32_t API_VERSION = base::config::RPC_PUBLIC_API_VERSION;
constexpr std::string_view VERSION = "0.1";
} // namespace config