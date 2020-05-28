#pragma once

#include "base/big_integer.hpp"
#include "base/bytes.hpp"

#include <cstdint>

namespace lk
{

// type that can be used to store any possible amount of currency
using Balance = base::Uint256;

using Fee = std::uint64_t;

using NonceInt = std::uint64_t;

using BlockDepth = std::uint64_t;

} // namespace lk
