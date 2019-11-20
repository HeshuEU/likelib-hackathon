#pragma once

#include "base/bytes.hpp"

namespace base
{

base::Bytes sha256(const base::Bytes& data);
base::Bytes sha1(const base::Bytes& data);

} // namespace base