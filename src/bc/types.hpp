#pragma once

#include "base/bytes.hpp"

#include <cstdint>

namespace bc
{

// type that can be used to store any possible amount of currency
using Balance = std::uint64_t;

using NonceInt = std::uint64_t;

} // namespace bc
