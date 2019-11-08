#pragma once

#include "bytes.hpp"

namespace base
{

template<typename I>
Bytes::Bytes(I begin, I end) : _raw(begin, end)
{}

} // namespace base